#include "hal/SimulatedRadioHal.h"

#include <QtTest>

class TestRadio : public QObject
{
    Q_OBJECT
private slots:
    void tuneAlignsAndFindsStation()
    {
        SimulatedRadioHal radio(nullptr, 1);
        radio.tune(89140);
        QCOMPARE(radio.frequencyKhz(), 89100);
        QCOMPARE(radio.stationName(), QStringLiteral("ROCK SIM"));
        QCOMPARE(radio.frequencyLabel(), QStringLiteral("89.1"));
    }

    void tuneClampsToBand()
    {
        SimulatedRadioHal radio(nullptr, 1);
        radio.tune(200000);
        QCOMPARE(radio.frequencyKhz(), 108000);
        QVERIFY(radio.stationName().isEmpty());
    }

    void seekUpStopsAtNextStation()
    {
        SimulatedRadioHal radio(nullptr, 1);
        radio.tune(90000);
        radio.seekUp();
        QVERIFY(radio.isSeeking());
        QTRY_VERIFY(!radio.isSeeking());
        QCOMPARE(radio.frequencyKhz(), 91300);
    }

    void seekUpWrapsAround()
    {
        SimulatedRadioHal radio(nullptr, 1);
        radio.tune(107000);
        radio.seekUp();
        QTRY_VERIFY(!radio.isSeeking());
        QCOMPARE(radio.frequencyKhz(), 89100);
    }

    void seekDownFindsPrevious()
    {
        SimulatedRadioHal radio(nullptr, 1);
        radio.tune(99000);
        radio.seekDown();
        QTRY_VERIFY(!radio.isSeeking());
        QCOMPARE(radio.frequencyKhz(), 97500);
    }

    void presetsSaveAndRecall()
    {
        SimulatedRadioHal radio(nullptr, 1);
        radio.tune(102100);
        radio.savePreset(5);
        radio.tune(88000);
        radio.recallPreset(5);
        QCOMPARE(radio.frequencyKhz(), 102100);
    }

    void switchToAm()
    {
        SimulatedRadioHal radio(nullptr, 1);
        radio.setBand(QStringLiteral("AM"));
        QCOMPARE(radio.band(), QStringLiteral("AM"));
        QCOMPARE(radio.frequencyKhz(), 640);
        QCOMPARE(radio.unit(), QStringLiteral("kHz"));
    }

    void scanListsStations()
    {
        SimulatedRadioHal radio(nullptr, 1);
        radio.scan();
        QTRY_COMPARE(radio.foundStations().size(), 8);
    }
};

QTEST_GUILESS_MAIN(TestRadio)
#include "tst_radio.moc"
