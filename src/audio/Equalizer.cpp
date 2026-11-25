#include "audio/Equalizer.h"

#include <QtGlobal>
#include <cmath>

Equalizer::Equalizer(QSettings *storage, QObject *parent)
    : QObject(parent), m_default(QStringLiteral("HeadUnit"), QStringLiteral("HeadUnit"))
{
    m_storage = storage ? storage : &m_default;
    const QVariantList saved = m_storage->value(QStringLiteral("eq/gains")).toList();
    if (saved.size() == BandCount)
        for (int i = 0; i < BandCount; ++i) m_gains[i] = qBound(-MaxGainDb, saved[i].toDouble(), MaxGainDb);
    m_preset = m_storage->value(QStringLiteral("eq/preset"), m_preset).toString();
    m_balance = m_storage->value(QStringLiteral("eq/balance"), 0.0).toDouble();
    m_fader = m_storage->value(QStringLiteral("eq/fader"), 0.0).toDouble();
    m_loudness = m_storage->value(QStringLiteral("eq/loudness"), false).toBool();
}

QVector<int> Equalizer::frequencies() { return {31, 62, 125, 250, 500, 1000, 2000, 4000, 8000, 16000}; }

QStringList Equalizer::presetNames()
{
    return {"Plano", "Pop", "Rock", "Jazz", "Clássico", "Eletrônico", "Voz", "Graves+"};
}

QVector<double> Equalizer::presetGains(const QString &name)
{
    if (name == u"Pop") return {-1, 1, 3, 4, 3, 0, -1, -1, 1, 2};
    if (name == u"Rock") return {5, 4, 2, -1, -2, -1, 2, 4, 5, 5};
    if (name == u"Jazz") return {3, 2, 1, 2, -1, -1, 0, 1, 2, 3};
    if (name == u"Clássico") return {4, 3, 2, 1, -1, -1, 0, 2, 3, 4};
    if (name == u"Eletrônico") return {6, 5, 2, 0, -2, 1, 0, 2, 4, 5};
    if (name == u"Voz") return {-3, -2, -1, 1, 3, 4, 4, 2, 0, -2};
    if (name == u"Graves+") return {7, 6, 4, 2, 0, 0, 0, 0, 0, 0};
    return QVector<double>(BandCount, 0.0); // Plano
}

void Equalizer::setGain(int band, double db)
{
    if (band < 0 || band >= BandCount) return;
    db = qBound(-MaxGainDb, std::round(db * 2) / 2, MaxGainDb); // passos de 0,5 dB
    if (qFuzzyCompare(db + 100, m_gains[band] + 100) && m_preset == u"Personalizado") return;
    m_gains[band] = db;
    m_preset = QStringLiteral("Personalizado");
    save();
}

void Equalizer::applyPreset(const QString &name)
{
    if (!presetNames().contains(name)) return;
    m_gains = presetGains(name);
    m_preset = name;
    save();
}

void Equalizer::reset()
{
    m_balance = 0;
    m_fader = 0;
    m_loudness = false;
    applyPreset(QStringLiteral("Plano"));
}

void Equalizer::setBalance(double v)
{
    v = qBound(-1.0, v, 1.0);
    if (qFuzzyCompare(v + 10, m_balance + 10)) return;
    m_balance = v;
    save();
}

void Equalizer::setFader(double v)
{
    v = qBound(-1.0, v, 1.0);
    if (qFuzzyCompare(v + 10, m_fader + 10)) return;
    m_fader = v;
    save();
}

void Equalizer::setLoudness(bool v)
{
    if (v == m_loudness) return;
    m_loudness = v;
    save();
}

double Equalizer::gainAt(double hz) const
{
    const QVector<int> f = frequencies();
    if (hz <= f.first()) return m_gains.first();
    if (hz >= f.last()) return m_gains.last();
    for (int i = 0; i + 1 < f.size(); ++i) {
        if (hz <= f[i + 1]) {
            const double t = (std::log(hz) - std::log(f[i])) / (std::log(f[i + 1]) - std::log(f[i]));
            return m_gains[i] + (m_gains[i + 1] - m_gains[i]) * t;
        }
    }
    return 0;
}

QVariantList Equalizer::frequenciesVariant() const
{
    QVariantList l;
    for (int hz : frequencies()) l << hz;
    return l;
}

QVariantList Equalizer::gainsVariant() const
{
    QVariantList l;
    for (double g : m_gains) l << g;
    return l;
}

void Equalizer::save()
{
    m_storage->setValue(QStringLiteral("eq/gains"), gainsVariant());
    m_storage->setValue(QStringLiteral("eq/preset"), m_preset);
    m_storage->setValue(QStringLiteral("eq/balance"), m_balance);
    m_storage->setValue(QStringLiteral("eq/fader"), m_fader);
    m_storage->setValue(QStringLiteral("eq/loudness"), m_loudness);
    emit changed();
}
