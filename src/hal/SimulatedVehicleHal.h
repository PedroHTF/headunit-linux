#pragma once

#include "hal/VehicleHal.h"

/** Carro de mentira, controlado pela janela "Simulador". */
class SimulatedVehicleHal : public VehicleHal
{
    Q_OBJECT
public:
    static constexpr double MaxSpeedKmh = 200.0;

    using VehicleHal::VehicleHal;

    QString name() const override { return QStringLiteral("Simulado"); }
    bool isSimulated() const override { return true; }

    /** "OFF", "ACC" ou "ON". */
    Q_INVOKABLE void setIgnitionByName(const QString &name);
    /** "P", "R", "N" ou "D". */
    Q_INVOKABLE void setGearByName(const QString &name);
    Q_INVOKABLE void setSpeed(double kmh);
    /** "previous", "playpause", "next", "voldown", "volup", "mute", "mode". */
    Q_INVOKABLE void pressKey(const QString &key);
};
