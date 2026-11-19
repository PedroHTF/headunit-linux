#include "hal/VehicleHal.h"

#include <QHash>

void VehicleHal::setState(Ignition ignition, Gear gear, double speedKmh)
{
    if (ignition == m_ignition && gear == m_gear && qFuzzyCompare(speedKmh + 1, m_speedKmh + 1))
        return;
    m_ignition = ignition;
    m_gear = gear;
    m_speedKmh = speedKmh;
    emit stateChanged();
}

void VehicleHal::setSteeringState(double deg)
{
    if (qFuzzyCompare(deg + 1000, m_steeringDeg + 1000)) return;
    m_steeringDeg = deg;
    emit stateChanged();
}

QString VehicleHal::ignitionLabel() const
{
    switch (m_ignition) {
    case Ignition::Off: return QStringLiteral("Desligado");
    case Ignition::Acc: return QStringLiteral("ACC");
    case Ignition::On: return QStringLiteral("Ligado");
    case Ignition::Start: return QStringLiteral("Partida");
    case Ignition::Unknown: break;
    }
    return QStringLiteral("Desconhecido");
}

QString VehicleHal::gearLabel() const { return gearToString(m_gear); }

QString VehicleHal::gearToString(Gear g)
{
    switch (g) {
    case Gear::Park: return QStringLiteral("P");
    case Gear::Reverse: return QStringLiteral("R");
    case Gear::Neutral: return QStringLiteral("N");
    case Gear::Drive: return QStringLiteral("D");
    case Gear::Unknown: break;
    }
    return QStringLiteral("-");
}

VehicleHal::Gear VehicleHal::gearFromString(const QString &s)
{
    const QString u = s.toUpper();
    if (u == u"P") return Gear::Park;
    if (u == u"R") return Gear::Reverse;
    if (u == u"N") return Gear::Neutral;
    if (u == u"D") return Gear::Drive;
    return Gear::Unknown;
}

static const QHash<QString, VehicleHal::Key> &keyNames()
{
    static const QHash<QString, VehicleHal::Key> names = {
        {QStringLiteral("previous"), VehicleHal::Key::Previous},
        {QStringLiteral("playpause"), VehicleHal::Key::PlayPause},
        {QStringLiteral("next"), VehicleHal::Key::Next},
        {QStringLiteral("voldown"), VehicleHal::Key::VolumeDown},
        {QStringLiteral("volup"), VehicleHal::Key::VolumeUp},
        {QStringLiteral("mute"), VehicleHal::Key::Mute},
        {QStringLiteral("mode"), VehicleHal::Key::Mode},
    };
    return names;
}

QString VehicleHal::keyToString(Key k) { return keyNames().key(k); }

bool VehicleHal::keyFromString(const QString &s, Key *out)
{
    const auto it = keyNames().constFind(s.toLower());
    if (it == keyNames().constEnd())
        return false;
    *out = it.value();
    return true;
}
