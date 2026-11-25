#pragma once

#include <QObject>
#include <QSettings>
#include <QVariantList>
#include <QVector>

/**
 * Equalizador de 10 bandas + balanço/fader + loudness, salvo entre execuções.
 * Mesmo modelo do headunit-android.
 *
 * Quem aplica: na central, o DSP de áudio (via MCU) ou um filtro do PipeWire no Linux.
 * No PC ainda não altera o som: guarda e mostra os valores.
 */
class Equalizer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList frequencies READ frequenciesVariant CONSTANT)
    Q_PROPERTY(QVariantList gains READ gainsVariant NOTIFY changed)
    Q_PROPERTY(QString preset READ preset NOTIFY changed)
    Q_PROPERTY(QStringList presets READ presetNames CONSTANT)
    Q_PROPERTY(double balance READ balance WRITE setBalance NOTIFY changed) // -1 esquerda .. +1 direita
    Q_PROPERTY(double fader READ fader WRITE setFader NOTIFY changed)       // -1 trás .. +1 frente
    Q_PROPERTY(bool loudness READ loudness WRITE setLoudness NOTIFY changed)

public:
    static constexpr int BandCount = 10;
    static constexpr double MaxGainDb = 12.0;

    explicit Equalizer(QSettings *storage = nullptr, QObject *parent = nullptr);

    static QVector<int> frequencies();
    static QStringList presetNames();
    static QVector<double> presetGains(const QString &name);

    QVector<double> gains() const { return m_gains; }
    QString preset() const { return m_preset; }
    double balance() const { return m_balance; }
    void setBalance(double v);
    double fader() const { return m_fader; }
    void setFader(double v);
    bool loudness() const { return m_loudness; }
    void setLoudness(bool v);

    Q_INVOKABLE void setGain(int band, double db);
    Q_INVOKABLE void applyPreset(const QString &name);
    Q_INVOKABLE void reset();

    /** Ganho (dB) numa frequência qualquer, interpolado em escala logarítmica: serve para
     *  equalizadores do sistema com outras bandas (ex.: 5 bandas no Android). */
    double gainAt(double hz) const;

signals:
    void changed();

private:
    QVariantList frequenciesVariant() const;
    QVariantList gainsVariant() const;
    void save();

    QSettings *m_storage;
    QSettings m_default;
    QVector<double> m_gains = QVector<double>(BandCount, 0.0);
    QString m_preset = QStringLiteral("Plano");
    double m_balance = 0;
    double m_fader = 0;
    bool m_loudness = false;
};
