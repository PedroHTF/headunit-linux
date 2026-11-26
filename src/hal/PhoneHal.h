#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVector>

struct Contact
{
    QString name;
    QString number;
};

struct RecentCall
{
    QString name;
    QString number;
    QString kind; // "in", "out" ou "missed"
    QDateTime when;
};

/**
 * Telefone via Bluetooth (o celular faz a ligação; a central é o viva-voz).
 *
 * Implementações:
 *  - SimulatedPhoneHal: celular de mentira, com agenda e chamadas simuladas.
 *  - OfonoPhoneHal (Linux): chamadas HFP pelo oFono/BlueZ via D-Bus + agenda pelo PBAP (obexd).
 */
class PhoneHal : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool simulated READ isSimulated CONSTANT)
    Q_PROPERTY(bool connected READ isConnected NOTIFY changed)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY changed)
    Q_PROPERTY(int battery READ battery NOTIFY changed)   // 0..100
    Q_PROPERTY(int signal READ signal NOTIFY changed)     // 0..5
    /** "idle", "incoming", "dialing" ou "active". */
    Q_PROPERTY(QString callState READ callState NOTIFY changed)
    Q_PROPERTY(QString callName READ callName NOTIFY changed)
    Q_PROPERTY(QString callNumber READ callNumber NOTIFY changed)
    Q_PROPERTY(int callSeconds READ callSeconds NOTIFY changed)
    Q_PROPERTY(bool muted READ isMuted NOTIFY changed)
    Q_PROPERTY(QVariantList contacts READ contactsVariant NOTIFY changed)
    Q_PROPERTY(QVariantList recents READ recentsVariant NOTIFY changed)
    /** Baixando a agenda do celular agora. */
    Q_PROPERTY(bool syncing READ isSyncing NOTIFY changed)
    /** Texto curto sobre a agenda ("124 contatos", "Erro: ..."); vazio no simulado. */
    Q_PROPERTY(QString phonebookStatus READ phonebookStatus NOTIFY changed)

public:
    using QObject::QObject;

    virtual QString name() const = 0;
    virtual bool isSimulated() const = 0;

    Q_INVOKABLE virtual void dial(const QString &number) = 0;
    Q_INVOKABLE virtual void accept() = 0;
    Q_INVOKABLE virtual void reject() = 0;
    Q_INVOKABLE virtual void hangup() = 0;
    Q_INVOKABLE virtual void setMuted(bool muted) = 0;
    /** Tom de tecla durante a chamada (menus de atendimento). */
    Q_INVOKABLE virtual void sendDtmf(const QString &digit) = 0;
    /** Baixa de novo a agenda e o histórico do celular (PBAP). */
    Q_INVOKABLE virtual void refreshPhonebook() {}

    bool isConnected() const { return m_connected; }
    QString deviceName() const { return m_deviceName; }
    int battery() const { return m_battery; }
    int signal() const { return m_signal; }
    QString callState() const { return m_callState; }
    bool inCall() const { return m_callState != u"idle"; }
    QString callName() const { return m_callName; }
    QString callNumber() const { return m_callNumber; }
    int callSeconds() const { return m_callSeconds; }
    bool isMuted() const { return m_muted; }
    bool isSyncing() const { return m_syncing; }
    QString phonebookStatus() const { return m_phonebookStatus; }
    QVector<Contact> contacts() const { return m_contacts; }
    QVector<RecentCall> recents() const { return m_recents; }

    /** Nome na agenda para um número (ou o próprio número). */
    Q_INVOKABLE QString nameFor(const QString &number) const;

signals:
    void changed();

protected:
    bool m_connected = false;
    QString m_deviceName;
    int m_battery = 0;
    int m_signal = 0;
    QString m_callState = QStringLiteral("idle");
    QString m_callName;
    QString m_callNumber;
    int m_callSeconds = 0;
    bool m_muted = false;
    bool m_syncing = false;
    QString m_phonebookStatus;
    QVector<Contact> m_contacts;
    QVector<RecentCall> m_recents;

private:
    QVariantList contactsVariant() const;
    QVariantList recentsVariant() const;
};
