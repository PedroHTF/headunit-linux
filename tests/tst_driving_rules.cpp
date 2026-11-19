#include "app/AppController.h"
#include "hal/DrivingRules.h"
#include "hal/SimulatedRadioHal.h"
#include "hal/SimulatedVehicleHal.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

class TestDrivingRules : public QObject
{
    Q_OBJECT
private slots:
    void videoOnlyWhenStopped()
    {
        QVERIFY(DrivingRules::isVideoAllowed(0));
        QVERIFY(!DrivingRules::isVideoAllowed(30));
        QVERIFY(DrivingRules::isVideoAllowed(120, true));
    }

    void reverseCamera()
    {
        using V = VehicleHal;
        QVERIFY(DrivingRules::shouldShowReverseCamera(V::Ignition::On, V::Gear::Reverse));
        QVERIFY(!DrivingRules::shouldShowReverseCamera(V::Ignition::On, V::Gear::Drive));
        QVERIFY(!DrivingRules::shouldShowReverseCamera(V::Ignition::Off, V::Gear::Reverse));
    }

    void controllerReactsToSimulator()
    {
        SimulatedVehicleHal vehicle;
        SimulatedRadioHal radio(nullptr, 1);
        MediaLibrary library;
        MediaPlayerController player(&library);
        QTemporaryDir dir;
        QSettings storage(dir.filePath(QStringLiteral("s.ini")), QSettings::IniFormat);
        AppSettings settings(&storage);
        AppController app(&vehicle, &radio, &player, &settings);

        vehicle.setGearByName(QStringLiteral("R"));
        QVERIFY(app.showReverseCamera());
        vehicle.setSpeed(40);
        QVERIFY(!app.videoAllowed());
        settings.setVideoWhileDriving(true);
        QVERIFY(app.videoAllowed());

        // Botão "Fonte" liga o rádio; ACC desligado desliga.
        vehicle.pressKey(QStringLiteral("mode"));
        QCOMPARE(app.audioSource(), QStringLiteral("radio"));
        QVERIFY(radio.isOn());
        vehicle.setIgnitionByName(QStringLiteral("OFF"));
        QVERIFY(!radio.isOn());
    }
};

QTEST_MAIN(TestDrivingRules)
#include "tst_driving_rules.moc"
