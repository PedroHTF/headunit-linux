#include "hal/RadioHal.h"

#include <QVariantMap>
#include <QtGlobal>
#include <cmath>

BandInfo RadioHal::bandInfo(const QString &band)
{
    if (band == u"AM")
        return {530, 1700, 10};
    return {87500, 108000, 100};
}

int RadioHal::align(const QString &band, int khz)
{
    const BandInfo b = bandInfo(band);
    const int clamped = qBound(b.minKhz, khz, b.maxKhz);
    const int steps = int(std::lround(double(clamped - b.minKhz) / b.stepKhz));
    return b.minKhz + steps * b.stepKhz;
}

QString RadioHal::format(const QString &band, int khz)
{
    if (band == u"AM")
        return QString::number(khz);
    return QString::number(khz / 1000.0, 'f', 1);
}

QVariantList RadioHal::presetsVariant() const
{
    QVariantList list;
    for (int f : m_presets)
        list << f;
    return list;
}

QVariantList RadioHal::foundStationsVariant() const
{
    QVariantList list;
    for (const RadioStation &s : m_found) {
        list << QVariantMap{
            {QStringLiteral("frequencyKhz"), s.frequencyKhz},
            {QStringLiteral("label"), format(s.band, s.frequencyKhz)},
            {QStringLiteral("name"), s.name},
        };
    }
    return list;
}
