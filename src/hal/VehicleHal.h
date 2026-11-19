#pragma once

#include <QObject>
#include <QString>

/**
 * Fonte dos dados do carro.
 *
 * Implementações:
 *  - SimulatedVehicleHal: controlada pela janela "Simulador" (desenvolvimento no PC).
 *  - futuramente: McuVehicleHal, falando com o MCU da central pela porta serial,
 *    e/ou SocketCAN para ler o barramento CAN.
 */
class VehicleHal : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool simulated READ isSimulated CONSTANT)
    Q_PROPERTY(QString ignitionLabel READ ignitionLabel NOTIFY stateChanged)
    Q_PROPERTY(QString gearLabel READ gearLabel NOTIFY stateChanged)
    Q_PROPERTY(double speedKmh READ speedKmh NOTIFY stateChanged)
    Q_PROPERTY(bool powered READ powered NOTIFY stateChanged)
    Q_PROPERTY(double steeringDeg READ steeringDeg NOTIFY stateChanged)

public:
    enum class Gear { Park, Reverse, Neutral, Drive, Unknown };
    Q_ENUM(Gear)
    enum class Ignition { Off, Acc, On, Start, Unknown };
    Q_ENUM(Ignition)
    enum class Key { Previous, PlayPause, Next, VolumeDown, VolumeUp, Mute, Mode };
    Q_ENUM(Key)

    using QObject::QObject;

    virtual QString name() const = 0;
    virtual bool isSimulated() const = 0;

    Ignition ignition() const { return m_ignition; }
    Gear gear() const { return m_gear; }
    double speedKmh() const { return m_speedKmh; }
    /** Ângulo do volante (graus, + = direita). Na central vem da caixa CAN. */
    double steeringDeg() const { return m_steeringDeg; }
    bool powered() const { return isPowered(m_ignition); }

    QString ignitionLabel() const;
    QString gearLabel() const;

    /** A central deve ficar ligada? (sem informação, assume que sim) */
    static bool isPowered(Ignition i) { return i != Ignition::Off; }

    static QString gearToString(Gear g);
    static Gear gearFromString(const QString &s);
    static QString keyToString(Key k);
    static bool keyFromString(const QString &s, Key *out);

signals:
    void stateChanged();
    void steeringKey(VehicleHal::Key key);

protected:
    void setState(Ignition ignition, Gear gear, double speedKmh);
    void setSteeringState(double deg);

private:
    Ignition m_ignition = Ignition::On;
    Gear m_gear = Gear::Park;
    double m_speedKmh = 0.0;
    double m_steeringDeg = 0.0;
};
