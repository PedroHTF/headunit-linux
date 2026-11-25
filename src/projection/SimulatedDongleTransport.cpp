#include "projection/SimulatedDongleTransport.h"

#include <QDateTime>
#include <QLinearGradient>
#include <QPainter>
#include <QtEndian>

using namespace dongle;

SimulatedDongleTransport::SimulatedDongleTransport(QObject *parent, int connectDelayMs) : DongleTransport(parent)
{
    m_connect.setSingleShot(true);
    m_connect.setInterval(connectDelayMs);
    connect(&m_connect, &QTimer::timeout, this, [this] { plugPhone(m_phone); });
    m_frames.setInterval(1000 / 15);
    connect(&m_frames, &QTimer::timeout, this, [this] { emit frameReady(render()); });
}

void SimulatedDongleTransport::start()
{
    if (m_running) return;
    m_running = true;
    QTimer::singleShot(200, this, [this] { if (m_running) emit opened(); });
}

void SimulatedDongleTransport::stop()
{
    m_running = false;
    m_connect.stop();
    m_frames.stop();
    m_plugged = false;
    emit closed();
}

void SimulatedDongleTransport::write(const QByteArray &msg)
{
    m_written << msg;
    const auto h = parseHeader(msg);
    if (!h) return;
    const QByteArray payload = msg.mid(HeaderSize);
    switch (h->type) {
    case Type::Open:
        m_width = int(qFromLittleEndian<uint32_t>(payload.constData()));
        m_height = int(qFromLittleEndian<uint32_t>(payload.constData() + 4));
        if (m_autoPlug && !m_plugged) m_connect.start(); // nos testes: celular conecta sozinho
        break;
    case Type::Touch: {
        const uint32_t action = qFromLittleEndian<uint32_t>(payload.constData());
        const double x = qFromLittleEndian<uint32_t>(payload.constData() + 4) / 10000.0;
        const double y = qFromLittleEndian<uint32_t>(payload.constData() + 8) / 10000.0;
        m_touch = action == uint32_t(TouchAction::Up) ? QPointF(-1, -1) : QPointF(x, y);
        if (action == uint32_t(TouchAction::Down)) {
            // Toque num "app" da grade 4x2.
            const int col = int(x * 4), row = int((y - 0.15) / 0.38);
            static const QStringList apps{"Mapas", "Música", "Telefone", "Mensagens",
                                          "Podcasts", "Agenda", "Audiolivros", "Ajustes"};
            if (y > 0.15 && row >= 0 && row < 2 && col >= 0 && col < 4) m_openApp = apps[row * 4 + col];
        }
        break;
    }
    case Type::Command:
        if (auto c = parseU32(payload)) onCommand(Command(*c));
        break;
    default:
        break;
    }
}

void SimulatedDongleTransport::onCommand(Command c)
{
    if (c == Command::Home) m_openApp.clear();
}

void SimulatedDongleTransport::plugPhone(PhoneType type)
{
    if (!m_running) return;
    m_phone = type;
    m_plugged = true;
    m_openApp.clear();
    m_since.start();
    QByteArray p;
    char buf[4];
    qToLittleEndian(uint32_t(type), buf);
    p.append(buf, 4);
    emit message(Type::Plugged, p);
    m_frames.start();
}

void SimulatedDongleTransport::unplugPhone()
{
    if (!m_plugged) return;
    m_plugged = false;
    m_frames.stop();
    emit message(Type::Unplugged, {});
}

QImage SimulatedDongleTransport::render() const
{
    const bool carplay = m_phone == PhoneType::CarPlay;
    QImage img(m_width, m_height, QImage::Format_RGB32);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient g(0, 0, 0, m_height);
    g.setColorAt(0, carplay ? QColor(30, 40, 70) : QColor(32, 33, 36));
    g.setColorAt(1, carplay ? QColor(10, 12, 25) : QColor(18, 18, 20));
    p.fillRect(img.rect(), g);

    QFont f = p.font();
    f.setPixelSize(m_height / 22);
    p.setFont(f);
    p.setPen(Qt::white);
    const QString clock = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm"));
    p.drawText(QRect(12, 6, m_width - 24, m_height / 12), Qt::AlignVCenter | Qt::AlignLeft,
               (carplay ? QStringLiteral("CarPlay (simulado)") : QStringLiteral("Android Auto (simulado)")));
    p.drawText(QRect(12, 6, m_width - 24, m_height / 12), Qt::AlignVCenter | Qt::AlignRight, clock);

    if (!m_openApp.isEmpty()) {
        f.setPixelSize(m_height / 10);
        p.setFont(f);
        p.drawText(img.rect(), Qt::AlignCenter, m_openApp);
        f.setPixelSize(m_height / 26);
        p.setFont(f);
        p.setPen(QColor(200, 200, 200));
        p.drawText(QRect(0, m_height * 6 / 10, m_width, m_height / 8), Qt::AlignCenter,
                   QStringLiteral("app aberto · botão Início do celular volta"));
    } else {
        static const QStringList apps{"Mapas", "Música", "Telefone", "Mensagens",
                                      "Podcasts", "Agenda", "Audiolivros", "Ajustes"};
        static const QList<QColor> colors{{52, 168, 83}, {234, 67, 53}, {66, 133, 244}, {251, 188, 5},
                                          {156, 39, 176}, {0, 150, 136}, {255, 112, 67}, {120, 144, 156}};
        const double cw = m_width / 4.0, ch = m_height * 0.38;
        for (int i = 0; i < apps.size(); ++i) {
            const int col = i % 4, row = i / 4;
            const QRectF cell(col * cw, m_height * 0.15 + row * ch, cw, ch);
            const double s = std::min(cw, ch) * 0.55;
            const QRectF icon(cell.center().x() - s / 2, cell.top() + ch * 0.12, s, s);
            p.setPen(Qt::NoPen);
            p.setBrush(colors[i]);
            if (carplay) p.drawRoundedRect(icon, s * 0.22, s * 0.22);
            else p.drawEllipse(icon);
            p.setPen(Qt::white);
            f.setPixelSize(m_height / 30);
            p.setFont(f);
            p.drawText(QRectF(cell.left(), icon.bottom() + 4, cw, ch * 0.2), Qt::AlignHCenter | Qt::AlignTop, apps[i]);
        }
    }
    if (m_touch.x() >= 0) {
        p.setPen(QPen(QColor(255, 255, 255, 200), 3));
        p.setBrush(QColor(255, 255, 255, 60));
        p.drawEllipse(QPointF(m_touch.x() * m_width, m_touch.y() * m_height), 26, 26);
    }
    return img;
}
