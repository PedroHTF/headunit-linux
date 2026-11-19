#include "hal/SimulatedRadioHal.h"

#include <algorithm>

SimulatedRadioHal::SimulatedRadioHal(QObject *parent, int seekStepMs, QVector<RadioStation> stations)
    : RadioHal(parent), m_stations(std::move(stations))
{
    m_seekTimer.setInterval(seekStepMs);
    connect(&m_seekTimer, &QTimer::timeout, this, &SimulatedRadioHal::onSeekTick);

    m_scanTimer.setSingleShot(true);
    m_scanTimer.setInterval(seekStepMs * 20);
    connect(&m_scanTimer, &QTimer::timeout, this, [this] {
        m_found.clear();
        for (const RadioStation &s : m_stations)
            if (s.band == m_band)
                m_found << s;
        std::sort(m_found.begin(), m_found.end(),
                  [](const RadioStation &a, const RadioStation &b) { return a.frequencyKhz < b.frequencyKhz; });
        m_seeking = false;
        emit changed();
    });

    int i = 0;
    for (const RadioStation &s : m_stations)
        if (s.band == u"FM" && i < PresetCount)
            m_presets[i++] = s.frequencyKhz;
    applyFrequency(bandInfo(m_band).minKhz);
}

QVector<RadioStation> SimulatedRadioHal::defaultStations()
{
    return {
        {"FM", 89100, "ROCK SIM", "Clássicos do rock 24h", 85},
        {"FM", 91300, "NOTICIAS", "Trânsito: marginal livre", 72},
        {"FM", 94700, "POP HITS", "Agora: as mais pedidas", 90},
        {"FM", 97500, "MPB FM", "Música brasileira", 64},
        {"FM", 99900, "SERTANEJ", "Modão sem parar", 78},
        {"FM", 102100, "JAZZ", "Jazz & Blues", 55},
        {"FM", 104500, "CLASSICA", "Concerto da tarde", 60},
        {"FM", 106300, "ESPORTES", "Futebol ao vivo", 81},
        {"AM", 640, "AM NEWS", "Jornal da manhã", 50},
        {"AM", 1020, "AM FUTEB", "Jogo ao vivo", 45},
    };
}

void SimulatedRadioHal::powerOn()
{
    if (m_on) return;
    m_on = true;
    emit changed();
}

void SimulatedRadioHal::powerOff()
{
    cancelSeek();
    if (!m_on) return;
    m_on = false;
    emit changed();
}

void SimulatedRadioHal::setBand(const QString &band)
{
    const QString b = band.toUpper() == u"AM" ? QStringLiteral("AM") : QStringLiteral("FM");
    if (b == m_band) return;
    cancelSeek();
    m_band = b;
    m_presets = QVector<int>(PresetCount, 0);
    m_found.clear();
    int first = bandInfo(b).minKhz;
    for (const RadioStation &s : m_stations)
        if (s.band == b) { first = s.frequencyKhz; break; }
    applyFrequency(first);
}

void SimulatedRadioHal::tune(int frequencyKhz)
{
    cancelSeek();
    applyFrequency(align(m_band, frequencyKhz));
}

void SimulatedRadioHal::stepUp() { step(+1); }
void SimulatedRadioHal::stepDown() { step(-1); }

void SimulatedRadioHal::step(int direction)
{
    tune(wrap(m_frequencyKhz + direction * bandInfo(m_band).stepKhz));
}

void SimulatedRadioHal::seekUp() { seek(+1); }
void SimulatedRadioHal::seekDown() { seek(-1); }

void SimulatedRadioHal::seek(int direction)
{
    cancelSeek();
    const BandInfo b = bandInfo(m_band);
    m_seekDirection = direction;
    m_seekStart = m_frequencyKhz;
    m_seekStepsLeft = (b.maxKhz - b.minKhz) / b.stepKhz + 1;
    m_seeking = true;
    emit changed();
    m_seekTimer.start();
}

void SimulatedRadioHal::onSeekTick()
{
    const int next = wrap(m_frequencyKhz + m_seekDirection * bandInfo(m_band).stepKhz);
    applyFrequency(next);
    if (stationAt(m_band, next)) {
        cancelSeek();
        return;
    }
    if (--m_seekStepsLeft <= 0) { // nenhuma estação: volta para onde estava
        applyFrequency(m_seekStart);
        cancelSeek();
    }
}

void SimulatedRadioHal::scan()
{
    cancelSeek();
    m_seeking = true;
    emit changed();
    m_scanTimer.start();
}

void SimulatedRadioHal::savePreset(int index)
{
    if (index < 0 || index >= PresetCount) return;
    m_presets[index] = m_frequencyKhz;
    emit changed();
}

void SimulatedRadioHal::recallPreset(int index)
{
    if (index < 0 || index >= PresetCount || m_presets[index] == 0) return;
    tune(m_presets[index]);
}

void SimulatedRadioHal::cancelSeek()
{
    m_seekTimer.stop();
    m_scanTimer.stop();
    if (m_seeking) {
        m_seeking = false;
        emit changed();
    }
}

void SimulatedRadioHal::applyFrequency(int khz)
{
    m_frequencyKhz = khz;
    if (const RadioStation *s = stationAt(m_band, khz)) {
        m_stationName = s->name;
        m_rdsText = s->rdsText;
        m_signal = s->signal;
    } else {
        m_stationName.clear();
        m_rdsText.clear();
        m_signal = (khz * 31) % 10; // ruído determinístico, 0..9
    }
    emit changed();
}

int SimulatedRadioHal::wrap(int khz) const
{
    const BandInfo b = bandInfo(m_band);
    if (khz > b.maxKhz) return b.minKhz;
    if (khz < b.minKhz) return b.maxKhz;
    return khz;
}

const RadioStation *SimulatedRadioHal::stationAt(const QString &band, int khz) const
{
    for (const RadioStation &s : m_stations)
        if (s.band == band && s.frequencyKhz == khz)
            return &s;
    return nullptr;
}
