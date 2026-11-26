#pragma once

#include "hal/PhoneHal.h"

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QTimer>
#include <QVariantMap>

class QDBusPendingCall;

/**
 * Agenda e histórico do celular pelo PBAP, usando o obexd do BlueZ
 * (serviço org.bluez.obex no D-Bus de SESSÃO do usuário).
 *
 * sync(endereço): cria a sessão PBAP, baixa a agenda ("pb") e o histórico ("cch"),
 * emite contactsReady/historyReady e fecha a sessão. Tudo assíncrono: nunca trava a tela.
 * O celular costuma pedir permissão ("Permitir acesso aos contatos?") na 1ª vez.
 */
class PbapClient : public QObject
{
    Q_OBJECT
public:
    explicit PbapClient(const QDBusConnection &sessionBus, QObject *parent = nullptr);
    ~PbapClient() override;

    void sync(const QString &address);
    void cancel();
    bool isBusy() const { return m_step != Step::Idle; }

    /** Quantas chamadas do histórico baixar. */
    static constexpr int kHistoryMax = 50;

signals:
    void contactsReady(const QVector<Contact> &contacts);
    void historyReady(const QVector<RecentCall> &history);
    void finished(bool ok, const QString &error);

public slots: // público: o QtDBus entrega sinais a ele
    void onTransferProperties(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

private:
    enum class Step { Idle, Session, SelectPb, PullPb, SelectCch, PullCch };

    void call(const QString &path, const QString &interface, const QString &method,
              const QVariantList &args, Step next);
    void onReply(QDBusPendingCall *call, Step step);
    void pull(const QString &phonebook);
    void watchTransfer(const QDBusObjectPath &transfer, const QVariantMap &props);
    void transferDone(bool ok);
    void fail(const QString &error);
    void closeSession();

    QDBusConnection m_bus;
    Step m_step = Step::Idle;
    QString m_session;
    QString m_transfer;
    QString m_transferFile;
    QTimer m_timeout;
    int m_generation = 0; // descarta respostas de um sync cancelado
};
