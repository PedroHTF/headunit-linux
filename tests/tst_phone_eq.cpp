#include "app/AppController.h"
#include "audio/Equalizer.h"
#include "hal/SimulatedPhoneHal.h"
#include "hal/SimulatedRadioHal.h"
#include "hal/SimulatedVehicleHal.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

class TestPhoneEq : public QObject
{
    Q_OBJECT
private slots:
    void outgoingCallIsAnsweredAndLogged()
    {
        SimulatedPhoneHal phone(nullptr, 10);
        const int before = phone.recents().size();
        phone.dial("+55 11 91234-5678");
        QCOMPARE(phone.callState(), QStringLiteral("dialing"));
        QCOMPARE(phone.callName(), QStringLiteral("Ana Souza")); // nome vem da agenda
        QTRY_COMPARE(phone.callState(), QStringLiteral("active"));
        phone.hangup();
        QCOMPARE(phone.callState(), QStringLiteral("idle"));
        QCOMPARE(phone.recents().size(), before + 1);
        QCOMPARE(phone.recents().first().kind, QStringLiteral("out"));
    }

    void rejectedIncomingIsMissed()
    {
        SimulatedPhoneHal phone;
        phone.simulateIncoming("+55 11 4002-8922");
        QCOMPARE(phone.callState(), QStringLiteral("incoming"));
        QCOMPARE(phone.callName(), QStringLiteral("Oficina"));
        phone.reject();
        QCOMPARE(phone.recents().first().kind, QStringLiteral("missed"));
    }

    void disconnectedPhoneCannotDial()
    {
        SimulatedPhoneHal phone;
        phone.setConnected(false);
        phone.dial("123");
        QCOMPARE(phone.callState(), QStringLiteral("idle"));
    }

    void callPausesRadioAndResumesAfter()
    {
        SimulatedVehicleHal vehicle;
        SimulatedRadioHal radio(nullptr, 1);
        MediaLibrary library;
        MediaPlayerController player(&library);
        QTemporaryDir dir;
        QSettings storage(dir.filePath("s.ini"), QSettings::IniFormat);
        AppSettings settings(&storage);
        SimulatedPhoneHal phone;
        AppController app(&vehicle, &radio, &player, &settings, &phone);

        app.onRadioStarted();
        radio.powerOn();
        phone.simulateIncoming("+55 11 98765-4321");
        QVERIFY(!radio.isOn());
        vehicle.pressKey("phone"); // atende pelo volante
        QCOMPARE(phone.callState(), QStringLiteral("active"));
        vehicle.pressKey("phone"); // desliga pelo volante
        QCOMPARE(phone.callState(), QStringLiteral("idle"));
        QVERIFY(radio.isOn());
    }

    void equalizerPresetsAndCustom()
    {
        QTemporaryDir dir;
        QSettings storage(dir.filePath("s.ini"), QSettings::IniFormat);
        Equalizer eq(&storage);
        QCOMPARE(eq.preset(), QStringLiteral("Plano"));
        eq.applyPreset("Rock");
        QCOMPARE(eq.gains().first(), 5.0);
        eq.setGain(0, 20); // limitado a +12
        QCOMPARE(eq.gains().first(), 12.0);
        QCOMPARE(eq.preset(), QStringLiteral("Personalizado"));

        Equalizer reloaded(&storage);
        QCOMPARE(reloaded.gains().first(), 12.0);
    }

    void equalizerInterpolatesOtherBands()
    {
        QTemporaryDir dir;
        QSettings storage(dir.filePath("s.ini"), QSettings::IniFormat);
        Equalizer eq(&storage);
        eq.setGain(5, 4);  // 1 kHz
        eq.setGain(6, 8);  // 2 kHz
        QCOMPARE(eq.gainAt(1000), 4.0);
        QVERIFY(std::abs(eq.gainAt(1414) - 6.0) < 0.05); // meio do caminho em escala log
        QCOMPARE(eq.gainAt(20), eq.gains().first());
    }
};

QTEST_MAIN(TestPhoneEq)
#include "tst_phone_eq.moc"
