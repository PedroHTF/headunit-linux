#include "hal/PbapClient.h"

#include "hal/VCard.h"

#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QFile>

namespace {
const QString kService = QStringLiteral("org.bluez.obex");
const QString kClientPath = QStringLiteral("/org/bluez/obex");
const QString kClientIface = QStringLiteral("org.bluez.obex.Client1");
const QString kPbapIface = QStringLiteral("org.bluez.obex.PhonebookAccess1");
const QString kTransferIface = QStringLiteral("org.bluez.obex.Transfer1");
const QString kPropsIface = QStringLiteral("org.freedesktop.DBus.Properties");
} // namespace

PbapClient::PbapClient(const QDBusConnection &sessionBus, QObject *parent)
    : QObject(parent), m_bus(sessionBus)
{
    m_timeout.setSingleShot(true);
    m_timeout.setInterval(90'000); // agendas grandes demoram; o celular pode estar esperando permissão
    connect(&m_timeout, &QTimer::timeout, this, [this] { fail(QStringLiteral("o celular não respondeu a tempo")); });
}

PbapClient::~PbapClient() { cancel(); }

void PbapClient::sync(const QString &address)
{
    cancel();
    ++m_generation;
    m_timeout.start();
    const QVariantMap opts{{QStringLiteral("Target"), QStringLiteral("PBAP")}};
    call(kClientPath, kClientIface, QStringLiteral("CreateSession"), {address, opts}, Step::Session);
}

void PbapClient::cancel()
{
    if (m_step == Step::Idle) return;
    ++m_generation;
    m_timeout.stop();
    closeSession();
    m_step = Step::Idle;
}

void PbapClient::call(const QString &path, const QString &interface, const QString &method,
                      const QVariantList &args, Step next)
{
    m_step = next;
    QDBusMessage msg = QDBusMessage::createMethodCall(kService, path, interface, method);
    msg.setArguments(args);
    auto *watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(msg, 60'000), this);
    const int gen = m_generation;
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, next, gen] {
        watcher->deleteLater();
        if (gen == m_generation) onReply(watcher, next);
    });
}

void PbapClient::onReply(QDBusPendingCall *pending, Step step)
{
    const QDBusMessage reply = pending->reply();
    if (reply.type() == QDBusMessage::ErrorMessage) {
        fail(reply.errorMessage().isEmpty() ? reply.errorName() : reply.errorMessage());
        return;
    }
    const QVariantList args = reply.arguments();
    switch (step) {
    case Step::Session:
        m_session = args.value(0).value<QDBusObjectPath>().path();
        call(m_session, kPbapIface, QStringLiteral("Select"), {QStringLiteral("int"), QStringLiteral("pb")}, Step::SelectPb);
        break;
    case Step::SelectPb:
        pull(QStringLiteral("pb"));
        break;
    case Step::SelectCch:
        pull(QStringLiteral("cch"));
        break;
    case Step::PullPb:
    case Step::PullCch: {
        QVariantMap props;
        if (args.size() > 1) props = qdbus_cast<QVariantMap>(args.at(1));
        watchTransfer(args.value(0).value<QDBusObjectPath>(), props);
        break;
    }
    case Step::Idle:
        break;
    }
}

void PbapClient::pull(const QString &phonebook)
{
    QVariantMap filters{{QStringLiteral("Format"), QStringLiteral("vcard30")}};
    if (phonebook != u"pb")
        filters.insert(QStringLiteral("MaxCount"), QVariant::fromValue<quint16>(kHistoryMax));
    // Arquivo de destino vazio: o obexd cria um temporário e informa em "Filename".
    call(m_session, kPbapIface, QStringLiteral("PullAll"), {QString(), filters},
         phonebook == u"pb" ? Step::PullPb : Step::PullCch);
}

void PbapClient::watchTransfer(const QDBusObjectPath &transfer, const QVariantMap &props)
{
    m_transfer = transfer.path();
    m_transferFile = props.value(QStringLiteral("Filename")).toString();
    m_bus.connect(kService, m_transfer, kPropsIface, QStringLiteral("PropertiesChanged"), this,
                  SLOT(onTransferProperties(QString,QVariantMap,QStringList)));
    // A transferência pode ter terminado antes de começarmos a ouvir: pergunta o estado.
    QDBusMessage get = QDBusMessage::createMethodCall(kService, m_transfer, kPropsIface, QStringLiteral("GetAll"));
    get.setArguments({kTransferIface});
    auto *watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(get), this);
    const int gen = m_generation;
    const QString path = m_transfer;
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, gen, path] {
        watcher->deleteLater();
        if (gen != m_generation || path != m_transfer) return;
        const QDBusMessage r = watcher->reply();
        // Erro aqui = objeto já sumiu, ou seja, já terminou; o sinal decide.
        if (r.type() == QDBusMessage::ReplyMessage && !r.arguments().isEmpty())
            onTransferProperties(kTransferIface, qdbus_cast<QVariantMap>(r.arguments().at(0)), {});
    });
}

void PbapClient::onTransferProperties(const QString &interface, const QVariantMap &changed, const QStringList &)
{
    if (interface != kTransferIface || m_transfer.isEmpty()) return;
    if (changed.contains(QStringLiteral("Filename")))
        m_transferFile = changed.value(QStringLiteral("Filename")).toString();
    const QString status = changed.value(QStringLiteral("Status")).toString();
    if (status == u"complete") transferDone(true);
    else if (status == u"error") transferDone(false);
}

void PbapClient::transferDone(bool ok)
{
    m_bus.disconnect(kService, m_transfer, kPropsIface, QStringLiteral("PropertiesChanged"), this,
                     SLOT(onTransferProperties(QString,QVariantMap,QStringList)));
    m_transfer.clear();
    QByteArray data;
    QFile file(m_transferFile);
    if (ok && file.open(QIODevice::ReadOnly)) data = file.readAll();
    file.close();
    if (!m_transferFile.isEmpty()) QFile::remove(m_transferFile); // temporário do obexd
    m_transferFile.clear();

    if (m_step == Step::PullPb) {
        if (!ok) { fail(QStringLiteral("falha ao baixar a agenda")); return; }
        emit contactsReady(VCard::parseContacts(data));
        call(m_session, kPbapIface, QStringLiteral("Select"), {QStringLiteral("int"), QStringLiteral("cch")}, Step::SelectCch);
    } else if (m_step == Step::PullCch) {
        // Histórico é opcional: alguns celulares não oferecem.
        if (ok) emit historyReady(VCard::parseHistory(data));
        m_timeout.stop();
        closeSession();
        m_step = Step::Idle;
        emit finished(true, QString());
    }
}

void PbapClient::fail(const QString &error)
{
    if (m_step == Step::Idle) return;
    // Agenda já veio e só o histórico falhou: conta como sucesso.
    const bool contactsDone = m_step == Step::SelectCch || m_step == Step::PullCch;
    ++m_generation;
    m_timeout.stop();
    if (!m_transfer.isEmpty())
        m_bus.disconnect(kService, m_transfer, kPropsIface, QStringLiteral("PropertiesChanged"), this,
                         SLOT(onTransferProperties(QString,QVariantMap,QStringList)));
    m_transfer.clear();
    closeSession();
    m_step = Step::Idle;
    qWarning().noquote() << "PBAP:" << error;
    emit finished(contactsDone, contactsDone ? QString() : error);
}

void PbapClient::closeSession()
{
    if (m_session.isEmpty()) return;
    QDBusMessage msg = QDBusMessage::createMethodCall(kService, kClientPath, kClientIface, QStringLiteral("RemoveSession"));
    msg.setArguments({QVariant::fromValue(QDBusObjectPath(m_session))});
    m_bus.asyncCall(msg);
    m_session.clear();
}
