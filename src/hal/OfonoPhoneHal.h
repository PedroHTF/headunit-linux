#pragma once

#include "hal/OfonoTypes.h"
#include "hal/PbapClient.h"
#include "hal/PhoneHal.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QMap>
#include <QTimer>

class QDBusServiceWatcher;

/**
 * Telefone real no Linux: a central faz o papel de viva-voz (HFP "Hands-Free") pelo oFono,
 * que conversa com o BlueZ. Agenda e histórico vêm do PBAP (obexd), via PbapClient.
 *
 *  oFono (D-Bus do SISTEMA, serviço org.ofono):
 *    /                      org.ofono.Manager          GetModems, ModemAdded/Removed
 *    /hfp/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX   (um "modem" por celular pareado)
 *                           org.ofono.Modem            Powered, Online, Name, Interfaces
 *                           org.ofono.VoiceCallManager Dial, GetCalls, CallAdded/Removed,
 *                                                      HoldAndAnswer, SendTones
 *                           org.ofono.CallVolume       Muted (microfone mudo)
 *                           org.ofono.NetworkRegistration  Strength (0..100)
 *                           org.ofono.Handsfree        BatteryChargeLevel (0..5)
 *    <modem>/voicecallNN    org.ofono.VoiceCall        Answer, Hangup, State, LineIdentification
 *
 * Tudo assíncrono. O áudio da chamada (SCO) é do PipeWire/WirePlumber: ver docs/telefone.md.
 */
class OfonoPhoneHal : public PhoneHal
{
    Q_OBJECT
public:
    OfonoPhoneHal(const QDBusConnection &systemBus, const QDBusConnection &sessionBus, QObject *parent = nullptr);

    QString name() const override { return QStringLiteral("Bluetooth (oFono)"); }
    bool isSimulated() const override { return false; }

    void dial(const QString &number) override;
    void accept() override;
    void reject() override;
    void hangup() override;
    void setMuted(bool muted) override;
    void sendDtmf(const QString &digit) override;
    void refreshPhonebook() override;

    /** O serviço org.ofono existe (ou pode ser ativado) neste barramento? */
    static bool isAvailable(const QDBusConnection &systemBus);

    struct CallInfo
    {
        QString state;  // estados do oFono: incoming, waiting, dialing, alerting, active, held, disconnected
        QString number;
        QString name;
        bool incoming = false;  // a ligação veio de fora
        bool wasActive = false; // chegou a ser atendida
    };
    /** Qual chamada a tela mostra: a que chega > a que está discando > a ativa > a em espera. */
    static QString focusCall(const QMap<QString, CallInfo> &calls);
    /** Estado do oFono -> estado da interface ("idle", "incoming", "dialing", "active"). */
    static QString uiState(const QString &ofonoState);
    /** "/hfp/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF" -> "AA:BB:CC:DD:EE:FF". */
    static QString addressFromPath(const QString &modemPath);

public slots:
    // Sinais do D-Bus. Públicos e recebendo a QDBusMessage inteira: o Qt 6.4 recusa conectar
    // sinais a slots com QDBusObjectPath/QDBusVariant nos parâmetros.
    void onServiceUp();
    void onServiceDown();
    void onManagerSignal(const QDBusMessage &msg);     // ModemAdded / ModemRemoved
    void onModemProperty(const QDBusMessage &msg);
    void onCallManagerSignal(const QDBusMessage &msg); // CallAdded / CallRemoved
    void onCallProperty(const QDBusMessage &msg);
    void onCallVolumeProperty(const QDBusMessage &msg);
    void onNetworkProperty(const QDBusMessage &msg);
    void onHandsfreeProperty(const QDBusMessage &msg);

private:
    void connectSignals();
    void loadModems();
    void chooseModem();
    void setModem(const QString &path);
    void loadModemDetails();
    void loadCalls();
    void getProperties(const QString &interface, void (OfonoPhoneHal::*apply)(const QString &, const QVariant &));
    void applyCallVolume(const QString &name, const QVariant &value);
    void applyNetwork(const QString &name, const QVariant &value);
    void applyHandsfree(const QString &name, const QVariant &value);
    void updateCall(const QString &path, const QVariantMap &props);
    void refresh();
    void updateConnection();
    void callModem(const QString &path, const QString &interface, const QString &method, const QVariantList &args = {});
    bool fromCurrentModem(const QDBusMessage &msg) const;
    void callAdded(const QString &path, const QVariantMap &props);
    void callRemoved(const QString &path);

    QDBusConnection m_system;
    PbapClient m_pbap;
    QDBusServiceWatcher *m_watcher = nullptr;
    QMap<QString, QVariantMap> m_modems; // caminho -> propriedades do org.ofono.Modem
    QString m_modem;
    QMap<QString, CallInfo> m_calls;
    QString m_focus;
    QTimer m_tick;
    QString m_syncedAddress;
    QVector<RecentCall> m_phoneHistory; // do PBAP
    QVector<RecentCall> m_localHistory; // chamadas desta sessão
};
