#include "projection/ProjectionController.h"

#include "projection/SimulatedDongleTransport.h"
#ifdef HEADUNIT_HAVE_LIBUSB
#include "projection/UsbDongleTransport.h"
#endif

#include <QAudioFormat>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMediaDevices>

using namespace dongle;

ProjectionController::ProjectionController(bool simulated, QObject *parent) : QObject(parent)
{
    m_heartbeat.setInterval(HeartbeatMs);
    connect(&m_heartbeat, &QTimer::timeout, this, [this] { if (m_transport) m_transport->write(heartbeat()); });
    m_fpsTimer.setInterval(1000);
    connect(&m_fpsTimer, &QTimer::timeout, this, [this] { m_fps = m_frames; m_frames = 0; });
    m_fpsTimer.start();
    createTransport(simulated || !usbAvailable());
}

ProjectionController::~ProjectionController() { stop(); }

bool ProjectionController::usbAvailable()
{
#ifdef HEADUNIT_HAVE_LIBUSB
    return true;
#else
    return false;
#endif
}

bool ProjectionController::isSimulated() const { return m_transport && m_transport->isSimulated(); }

void ProjectionController::createTransport(bool simulated)
{
    if (m_transport) {
        m_transport->stop();
        m_transport->deleteLater();
    }
#ifdef HEADUNIT_HAVE_LIBUSB
    if (!simulated) m_transport = new UsbDongleTransport(this);
    else
#endif
        m_transport = new SimulatedDongleTransport(this);
    Q_UNUSED(simulated);
    connect(m_transport, &DongleTransport::opened, this, &ProjectionController::onOpened);
    connect(m_transport, &DongleTransport::closed, this, [this] {
        m_heartbeat.stop();
        setState(QStringLiteral("searching"));
    });
    connect(m_transport, &DongleTransport::message, this, &ProjectionController::onMessage);
    connect(m_transport, &DongleTransport::frameReady, this, &ProjectionController::setFrame);
    connect(m_transport, &DongleTransport::error, this, [this](const QString &e) {
        m_error = e;
        emit stateChanged();
    });
    setState(QStringLiteral("searching"));
    if (m_started) m_transport->start();
}

void ProjectionController::setSimulated(bool simulated)
{
    if (simulated == isSimulated()) return;
    createTransport(simulated);
}

void ProjectionController::setVideoSize(int w, int h)
{
    m_config.width = w;
    m_config.height = h;
}

void ProjectionController::start()
{
    m_started = true;
    m_transport->start();
}

void ProjectionController::stop()
{
    m_started = false;
    m_heartbeat.stop();
    if (m_transport) m_transport->stop();
}

void ProjectionController::setState(const QString &s)
{
    if (s == m_state) return;
    m_state = s;
    if (s != u"connected") {
        m_phoneType.clear();
        m_frame = QImage();
        emit frameChanged();
    }
    emit stateChanged();
}

void ProjectionController::onOpened()
{
    m_error.clear();
    setState(QStringLiteral("ready"));
    for (const QByteArray &m : startupSequence(m_config, QDateTime::currentMSecsSinceEpoch()))
        m_transport->write(m);
    QTimer::singleShot(1000, this, [this] {
        if (m_transport && m_state != u"searching") m_transport->write(command(Command::WifiConnect));
    });
    m_heartbeat.start();
}

void ProjectionController::onMessage(Type type, const QByteArray &payload)
{
    switch (type) {
    case Type::Plugged:
        if (auto t = parsePlugged(payload)) m_phoneType = phoneTypeName(*t);
        m_state.clear(); // força o aviso mesmo se já estava "connected"
        setState(QStringLiteral("connected"));
        break;
    case Type::Unplugged:
        setState(QStringLiteral("ready"));
        break;
    case Type::VideoData:
        if (auto v = parseVideo(payload)) {
            const QImage img = m_decoder.decode(v->h264);
            if (!img.isNull()) setFrame(img);
        }
        break;
    case Type::AudioData:
        if (auto a = parseAudio(payload)) {
            if (a->command == AudioCommand::MediaStart || a->command == AudioCommand::OutputStart)
                emit mediaAudioStarted();
            if (!a->pcm.isEmpty()) playAudio(*a);
        }
        break;
    case Type::MediaData:
        if (payload.size() > 5 && parseU32(payload) == 1u) { // JSON com música/artista
            const QJsonObject o = QJsonDocument::fromJson(payload.mid(4, payload.size() - 5)).object();
            if (o.contains("MediaSongName")) m_mediaTitle = o.value("MediaSongName").toString();
            if (o.contains("MediaArtistName")) m_mediaArtist = o.value("MediaArtistName").toString();
            emit mediaChanged();
        }
        break;
    default:
        break;
    }
}

void ProjectionController::setFrame(const QImage &img)
{
    m_frame = img;
    ++m_frames;
    emit frameChanged();
}

void ProjectionController::playAudio(const AudioPacket &a)
{
    const auto fmt = audioFormat(a.decodeType);
    if (!fmt) return;
    Sink &s = m_sinks[a.audioType];
    if (!s.sink || s.decodeType != a.decodeType) {
        QAudioFormat f;
        f.setSampleRate(fmt->sampleRate);
        f.setChannelCount(fmt->channels);
        f.setSampleFormat(QAudioFormat::Int16);
        s.sink = std::make_unique<QAudioSink>(QMediaDevices::defaultAudioOutput(), f);
        s.io = s.sink->start();
        s.decodeType = a.decodeType;
    }
    if (s.io) s.io->write(a.pcm);
}

void ProjectionController::touch(const QString &action, double x, double y)
{
    if (!isConnected()) return;
    const TouchAction t = action == u"down" ? TouchAction::Down : action == u"up" ? TouchAction::Up : TouchAction::Move;
    m_transport->write(dongle::touch(t, x, y));
}

void ProjectionController::key(const QString &name)
{
    static const QHash<QString, Command> map{
        {"home", Command::Home}, {"back", Command::Back}, {"siri", Command::Siri},
        {"playpause", Command::PlayOrPause}, {"next", Command::Next}, {"prev", Command::Prev},
        {"accept", Command::AcceptPhone}, {"reject", Command::RejectPhone},
    };
    const auto it = map.constFind(name);
    if (it != map.constEnd() && isConnected()) m_transport->write(command(*it));
}

void ProjectionController::simulatePhone(const QString &type)
{
    auto *sim = qobject_cast<SimulatedDongleTransport *>(m_transport.data());
    if (!sim) return;
    if (type.isEmpty()) sim->unplugPhone();
    else sim->plugPhone(type == u"androidauto" ? PhoneType::AndroidAuto : PhoneType::CarPlay);
}
