#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVector>

struct RadioStation
{
    QString band; // "FM" ou "AM"
    int frequencyKhz = 0;
    QString name;
    QString rdsText;
    int signal = 80; // 0..100
};

struct BandInfo
{
    int minKhz;
    int maxKhz;
    int stepKhz;
};

/**
 * Sintonizador de rádio. Frequências sempre em kHz.
 *
 * Implementações:
 *  - SimulatedRadioHal: estações falsas, para desenvolvimento no PC.
 *  - futuramente: chip do rádio da central (TEF668x, SAF775x...) via MCU/I2C,
 *    ou um dongle RTL-SDR no PC para ouvir FM de verdade.
 */
class RadioHal : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool simulated READ isSimulated CONSTANT)
    Q_PROPERTY(bool on READ isOn NOTIFY changed)
    Q_PROPERTY(QString band READ band NOTIFY changed)
    Q_PROPERTY(int frequencyKhz READ frequencyKhz NOTIFY changed)
    Q_PROPERTY(QString frequencyLabel READ frequencyLabel NOTIFY changed)
    Q_PROPERTY(QString unit READ unit NOTIFY changed)
    Q_PROPERTY(QString stationName READ stationName NOTIFY changed)
    Q_PROPERTY(QString rdsText READ rdsText NOTIFY changed)
    Q_PROPERTY(int signal READ signal NOTIFY changed)
    Q_PROPERTY(bool seeking READ isSeeking NOTIFY changed)
    Q_PROPERTY(QVariantList presets READ presetsVariant NOTIFY changed)
    Q_PROPERTY(QVariantList foundStations READ foundStationsVariant NOTIFY changed)

public:
    static constexpr int PresetCount = 6;

    using QObject::QObject;

    virtual QString name() const = 0;
    virtual bool isSimulated() const = 0;

    Q_INVOKABLE virtual void powerOn() = 0;
    Q_INVOKABLE virtual void powerOff() = 0;
    Q_INVOKABLE virtual void setBand(const QString &band) = 0;
    Q_INVOKABLE virtual void tune(int frequencyKhz) = 0;
    Q_INVOKABLE virtual void stepUp() = 0;
    Q_INVOKABLE virtual void stepDown() = 0;
    Q_INVOKABLE virtual void seekUp() = 0;
    Q_INVOKABLE virtual void seekDown() = 0;
    Q_INVOKABLE virtual void scan() = 0;
    Q_INVOKABLE virtual void savePreset(int index) = 0;
    Q_INVOKABLE virtual void recallPreset(int index) = 0;

    Q_INVOKABLE QString formatFrequency(int khz) const { return format(m_band, khz); }

    bool isOn() const { return m_on; }
    QString band() const { return m_band; }
    int frequencyKhz() const { return m_frequencyKhz; }
    QString frequencyLabel() const { return format(m_band, m_frequencyKhz); }
    QString unit() const { return m_band == u"FM" ? QStringLiteral("MHz") : QStringLiteral("kHz"); }
    QString stationName() const { return m_stationName; }
    QString rdsText() const { return m_rdsText; }
    int signal() const { return m_signal; }
    bool isSeeking() const { return m_seeking; }
    QVector<int> presets() const { return m_presets; }
    QVector<RadioStation> foundStations() const { return m_found; }

    static BandInfo bandInfo(const QString &band);
    static int align(const QString &band, int khz);
    static QString format(const QString &band, int khz);

signals:
    void changed();

protected:
    bool m_on = false;
    QString m_band = QStringLiteral("FM");
    int m_frequencyKhz = 87500;
    QString m_stationName;
    QString m_rdsText;
    int m_signal = 0;
    bool m_seeking = false;
    QVector<int> m_presets = QVector<int>(PresetCount, 0); // 0 = memória vazia
    QVector<RadioStation> m_found;

private:
    QVariantList presetsVariant() const;
    QVariantList foundStationsVariant() const;
};
