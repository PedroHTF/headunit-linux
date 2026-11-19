#include "hal/SimulatedVehicleHal.h"

#include <QtGlobal>

void SimulatedVehicleHal::setIgnitionByName(const QString &name)
{
    const QString u = name.toUpper();
    Ignition i = Ignition::Unknown;
    if (u == u"OFF") i = Ignition::Off;
    else if (u == u"ACC") i = Ignition::Acc;
    else if (u == u"ON") i = Ignition::On;
    setState(i, gear(), speedKmh());
}

void SimulatedVehicleHal::setGearByName(const QString &name)
{
    setState(ignition(), gearFromString(name), speedKmh());
}

void SimulatedVehicleHal::setSpeed(double kmh)
{
    setState(ignition(), gear(), qBound(0.0, kmh, MaxSpeedKmh));
}

void SimulatedVehicleHal::pressKey(const QString &key)
{
    Key k;
    if (keyFromString(key, &k))
        emit steeringKey(k);
}
