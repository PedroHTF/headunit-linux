#include "app/AppController.h"
#include "hal/SimulatedRadioHal.h"
#include "hal/SimulatedVehicleHal.h"
#include "projection/DongleProtocol.h"
#include "projection/ProjectionController.h"
#include "projection/SimulatedDongleTransport.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>

using namespace dongle;

static uint32_t u32(const QByteArray &b, int off) { return qFromLittleEndian<uint32_t>(b.constData() + off); }

class TestDongle : public QObject
{
    Q_OBJECT
private slots:
    void headerLayout()
    {
        const QByteArray m = heartbeat();
        QCOMPARE(m.size(), 16);
        QCOMPARE(u32(m, 0), 0x55AA55AAu);
        QCOMPARE(u32(m, 4), 0u);
        QCOMPARE(u32(m, 8), 0xAAu);
        QCOMPARE(u32(m, 12), 0xFFFFFF55u); // ~0xAA
        const auto h = parseHeader(m);
        QVERIFY(h);
        QCOMPARE(h->type, Type::HeartBeat);
    }

    void rejectsBadHeader()
    {
        QByteArray m = heartbeat();
        m[12] = 0; // type check errado
        QVERIFY(!parseHeader(m));
        QVERIFY(!parseHeader(QByteArray(8, 0)));
    }

    void openPayload()
    {
        Config c;
        c.width = 864; c.height = 560; c.fps = 30;
        const QByteArray m = open(c);
        QCOMPARE(u32(m, 4), 28u);
        const QByteArray p = m.mid(16);
        QCOMPARE(u32(p, 0), 864u);
        QCOMPARE(u32(p, 4), 560u);
        QCOMPARE(u32(p, 8), 30u);
        QCOMPARE(u32(p, 12), 5u);      // format
        QCOMPARE(u32(p, 16), 49152u);  // packetMax
        QCOMPARE(u32(p, 20), 2u);      // iBoxVersion
        QCOMPARE(u32(p, 24), 2u);      // phoneWorkMode
    }

    void touchScaling()
    {
        const QByteArray p = touch(TouchAction::Down, 0.5, 1.5).mid(16);
        QCOMPARE(u32(p, 0), 14u);
        QCOMPARE(u32(p, 4), 5000u);
        QCOMPARE(u32(p, 8), 10000u); // limitado
        QCOMPARE(u32(p, 12), 0u);
    }

    void sendFileLayout()
    {
        const QByteArray p = sendNumber("/tmp/screen_dpi", 160).mid(16);
        QCOMPARE(u32(p, 0), 16u); // "/tmp/screen_dpi" (15) + \0
        QCOMPARE(p.mid(4, 15), QByteArray("/tmp/screen_dpi"));
        QCOMPARE(p.at(19), '\0');
        QCOMPARE(u32(p, 20), 4u);
        QCOMPARE(u32(p, 24), 160u);
    }

    void startupStartsWithDpiThenOpen()
    {
        const auto seq = startupSequence(Config{}, 0);
        QCOMPARE(seq.size(), 11);
        QCOMPARE(parseHeader(seq[0])->type, Type::SendFile);
        QCOMPARE(parseHeader(seq[1])->type, Type::Open);
        QCOMPARE(u32(seq.last().mid(16), 0), uint32_t(Command::AudioTransferOff));
    }

    void parseReceived()
    {
        QByteArray v;
        for (uint32_t x : {800u, 480u, 0u, 3u, 0u}) { char b[4]; qToLittleEndian(x, b); v.append(b, 4); }
        v.append("\x00\x00\x01", 3);
        const auto frame = parseVideo(v);
        QVERIFY(frame);
        QCOMPARE(frame->width, 800u);
        QCOMPARE(frame->h264.size(), 3);

        QByteArray a;
        for (uint32_t x : {4u, 0u, 1u}) { char b[4]; qToLittleEndian(x, b); a.append(b, 4); }
        a.append(char(AudioCommand::MediaStart));
        const auto audio = parseAudio(a);
        QVERIFY(audio && audio->command == AudioCommand::MediaStart);
        QCOMPARE(audioFormat(4)->sampleRate, 48000);
        QCOMPARE(audioFormat(5)->channels, 1);

        QByteArray pl; char b[4]; qToLittleEndian(5u, b); pl.append(b, 4);
        QCOMPARE(*parsePlugged(pl), PhoneType::AndroidAuto);
    }

    void simulatedSessionConnectsAndTouches()
    {
        ProjectionController proj(true);
        proj.setVideoSize(400, 240);
        proj.start();
        QTRY_COMPARE(proj.state(), QStringLiteral("ready")); // dongle aberto, sem celular
        proj.simulatePhone("carplay");
        QCOMPARE(proj.state(), QStringLiteral("connected"));
        QCOMPARE(proj.phoneType(), QStringLiteral("CarPlay"));
        QTRY_VERIFY(!proj.frame().isNull());
        QCOMPARE(proj.frame().width(), 400);

        auto *sim = qobject_cast<SimulatedDongleTransport *>(proj.transport());
        QVERIFY(sim);
        proj.touch("down", 0.1, 0.3);
        const auto h = parseHeader(sim->written().last());
        QCOMPARE(h->type, Type::Touch);

        proj.simulatePhone("");
        QCOMPARE(proj.state(), QStringLiteral("ready"));
        proj.simulatePhone("androidauto");
        QCOMPARE(proj.phoneType(), QStringLiteral("Android Auto"));
    }

    void steeringKeysGoToPhoneWhenItPlays()
    {
        SimulatedVehicleHal vehicle;
        SimulatedRadioHal radio(nullptr, 1);
        MediaLibrary library;
        MediaPlayerController player(&library);
        QTemporaryDir dir;
        QSettings storage(dir.filePath("s.ini"), QSettings::IniFormat);
        AppSettings settings(&storage);
        AppController app(&vehicle, &radio, &player, &settings);
        ProjectionController proj(true);
        app.setProjection(&proj);
        qobject_cast<SimulatedDongleTransport *>(proj.transport())->setAutoPlug(true);
        proj.start();
        QTRY_COMPARE(proj.state(), QStringLiteral("connected")); // Open → celular conecta sozinho

        // O celular avisa que começou a tocar mídia.
        QByteArray a;
        for (uint32_t x : {4u, 0u, 1u}) { char b[4]; qToLittleEndian(x, b); a.append(b, 4); }
        a.append(char(AudioCommand::MediaStart));
        emit proj.transport()->message(Type::AudioData, a);
        QCOMPARE(app.audioSource(), QStringLiteral("projection"));

        vehicle.pressKey("next");
        auto *sim = qobject_cast<SimulatedDongleTransport *>(proj.transport());
        const QByteArray last = sim->written().last();
        QCOMPARE(parseHeader(last)->type, Type::Command);
        QCOMPARE(u32(last.mid(16), 0), uint32_t(Command::Next));
    }
};

QTEST_MAIN(TestDongle)
#include "tst_dongle.moc"
