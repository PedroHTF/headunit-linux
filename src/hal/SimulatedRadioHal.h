#pragma once

#include "hal/RadioHal.h"

#include <QTimer>

/**
 * Rádio falso: lista fixa de estações (nomes fictícios), com busca que leva tempo,
 * como num sintonizador de verdade. Não produz áudio.
 */
class SimulatedRadioHal : public RadioHal
{
    Q_OBJECT
public:
    explicit SimulatedRadioHal(QObject *parent = nullptr, int seekStepMs = 20,
                               QVector<RadioStation> stations = defaultStations());

    static QVector<RadioStation> defaultStations();

    QString name() const override { return QStringLiteral("Rádio simulado"); }
    bool isSimulated() const override { return true; }

    void powerOn() override;
    void powerOff() override;
    void setBand(const QString &band) override;
    void tune(int frequencyKhz) override;
    void stepUp() override;
    void stepDown() override;
    void seekUp() override;
    void seekDown() override;
    void scan() override;
    void savePreset(int index) override;
    void recallPreset(int index) override;

private:
    void step(int direction);
    void seek(int direction);
    void onSeekTick();
    void cancelSeek();
    void applyFrequency(int khz);
    int wrap(int khz) const;
    const RadioStation *stationAt(const QString &band, int khz) const;

    QVector<RadioStation> m_stations;
    QTimer m_seekTimer;
    QTimer m_scanTimer;
    int m_seekDirection = 0;
    int m_seekStart = 0;
    int m_seekStepsLeft = 0;
};
