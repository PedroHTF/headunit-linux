#include "hal/OfonoPhoneHal.h"

#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QRegularExpression>

namespace {
const QString kService = QStringLiteral("org.ofono");
const QString kManager = QStringLiteral("org.ofono.Manager");
const QString kModem = QStringLiteral("org.ofono.Modem");
const QString kVcm = QStringLiteral("org.ofono.VoiceCallManager");
const QString kCall = QStringLiteral("org.ofono.VoiceCall");
const QString kVolume = QStringLiteral("org.ofono.CallVolume");
const QString kNetwork = QStringLiteral("org.ofono.NetworkRegistration");
const QString kHandsfree = QStringLiteral("org.ofono.Handsfree");
constexpr int kMaxRecents = 50;

QStringList interfacesOf(const QVariantMap &modem) { return modem.value(QStringLiteral("Interfaces")).toStringList(); }
bool powered(const QVariantMap &modem) { return modem.value(QStringLiteral("Powered")).toBool(); }

// Argumentos de sinais: (o), (oa{sv}) e (sv).
QString pathArg(const QDBusMessage &m) { return m.arguments().value(0).value<QDBusObjectPath>().path(); }
QVariantMap mapArg(const QDBusMessage &m, int i) { return qdbus_cast<QVariantMap>(m.arguments().value(i)); }
QString nameArg(const QDBusMessage &m) { return m.arguments().value(0).toString(); }
QVariant valueArg(const QDBusMessage &m) { return m.arguments().value(1).value<QDBusVariant>().variant(); }
} // namespace

OfonoPhoneHal::OfonoPhoneHal(const QDBusConnection &systemBus, const QDBusConnection &sessionBus, QObject *parent)
    : PhoneHal(parent), m_system(systemBus), m_pbap(sessionBus)
{
    registerOfonoTypes();
    m_tick.setInterval(1000);
    connect(&m_tick, &QTimer::timeout, this, [this] {
        ++m_callSeconds;
        emit changed();
    });

    connect(&m_pbap, &PbapClient::contactsReady, this, [this](const QVector<Contact> &contacts) {
        m_contacts = contacts;
        if (!m_callNumber.isEmpty() && m_calls.value(m_focus).name.isEmpty()) m_callName = nameFor(m_callNumber);
        emit changed();
    });
    connect(&m_pbap, &PbapClient::historyReady, this, [this](const QVector<RecentCall> &history) {
        m_phoneHistory = history;
        m_localHistory.clear(); // o histórico do celular já inclui as chamadas desta viagem
        refresh();
    });
    connect(&m_pbap, &PbapClient::finished, this, [this](bool ok, const QString &error) {
        m_syncing = false;
        m_phonebookStatus = ok ? QStringLiteral("%1 contatos").arg(m_contacts.size())
                               : QStringLiteral("Agenda indisponível: %1").arg(error);
        emit changed();
    });

    m_watcher = new QDBusServiceWatcher(kService, m_system,
                                        QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration, this);
    connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this, &OfonoPhoneHal::onServiceUp);
    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, &OfonoPhoneHal::onServiceDown);
    connectSignals();
    loadModems();
}

bool OfonoPhoneHal::isAvailable(const QDBusConnection &systemBus)
{
    if (!systemBus.isConnected() || !systemBus.interface()) return false;
    if (systemBus.interface()->isServiceRegistered(kService).value()) return true;
    const QDBusReply<QStringList> activatable = systemBus.interface()->call(QStringLiteral("ListActivatableNames"));
    return activatable.isValid() && activatable.value().contains(kService);
}

// ---- Sinais do oFono (qualquer caminho; filtramos pelo modem escolhido) ------------------

void OfonoPhoneHal::connectSignals()
{
    const QString any; // caminho vazio = todos os objetos do serviço
    const QString changedSig = QStringLiteral("PropertyChanged");
    m_system.connect(kService, QStringLiteral("/"), kManager, QStringLiteral("ModemAdded"), this, SLOT(onManagerSignal(QDBusMessage)));
    m_system.connect(kService, QStringLiteral("/"), kManager, QStringLiteral("ModemRemoved"), this, SLOT(onManagerSignal(QDBusMessage)));
    m_system.connect(kService, any, kModem, changedSig, this, SLOT(onModemProperty(QDBusMessage)));
    m_system.connect(kService, any, kVcm, QStringLiteral("CallAdded"), this, SLOT(onCallManagerSignal(QDBusMessage)));
    m_system.connect(kService, any, kVcm, QStringLiteral("CallRemoved"), this, SLOT(onCallManagerSignal(QDBusMessage)));
    m_system.connect(kService, any, kCall, changedSig, this, SLOT(onCallProperty(QDBusMessage)));
    m_system.connect(kService, any, kVolume, changedSig, this, SLOT(onCallVolumeProperty(QDBusMessage)));
    m_system.connect(kService, any, kNetwork, changedSig, this, SLOT(onNetworkProperty(QDBusMessage)));
    m_system.connect(kService, any, kHandsfree, changedSig, this, SLOT(onHandsfreeProperty(QDBusMessage)));
}

bool OfonoPhoneHal::fromCurrentModem(const QDBusMessage &msg) const
{
    if (m_modem.isEmpty()) return false;
    const QString path = msg.path();
    return path == m_modem || path.startsWith(m_modem + u'/');
}

void OfonoPhoneHal::onServiceUp() { loadModems(); }

void OfonoPhoneHal::onServiceDown()
{
    m_modems.clear();
    setModem(QString());
}

void OfonoPhoneHal::loadModems()
{
    const QDBusMessage msg = QDBusMessage::createMethodCall(kService, QStringLiteral("/"), kManager, QStringLiteral("GetModems"));
    auto *w = new QDBusPendingCallWatcher(m_system.asyncCall(msg), this);
    connect(w, &QDBusPendingCallWatcher::finished, this, [this, w] {
        w->deleteLater();
        const QDBusMessage r = w->reply();
        if (r.type() != QDBusMessage::ReplyMessage || r.arguments().isEmpty()) return; // oFono não está rodando
        m_modems.clear();
        for (const OfonoObject &o : qdbus_cast<OfonoObjectList>(r.arguments().at(0)))
            m_modems.insert(o.path.path(), o.properties);
        chooseModem();
    });
}

void OfonoPhoneHal::onManagerSignal(const QDBusMessage &msg)
{
    if (msg.member() == u"ModemAdded") m_modems.insert(pathArg(msg), mapArg(msg, 1));
    else m_modems.remove(pathArg(msg));
    chooseModem();
}

void OfonoPhoneHal::onModemProperty(const QDBusMessage &msg)
{
    const QString path = msg.path();
    const QString name = nameArg(msg);
    if (!m_modems.contains(path)) return;
    m_modems[path].insert(name, valueArg(msg));
    if (path == m_modem && name == u"Interfaces") loadModemDetails(); // interfaces novas (ex.: VoiceCallManager)
    chooseModem();
}

/** Prefere o celular que está conectado de fato; senão, o primeiro pareado (HFP). */
void OfonoPhoneHal::chooseModem()
{
    QString best;
    int bestScore = -1;
    for (auto it = m_modems.cbegin(); it != m_modems.cend(); ++it) {
        const QVariantMap &m = it.value();
        const QString type = m.value(QStringLiteral("Type")).toString();
        if (!type.isEmpty() && type != u"hfp") continue; // só celulares via Bluetooth
        int score = 0;
        if (powered(m)) score += 2;
        if (interfacesOf(m).contains(kVcm)) score += 1;
        if (it.key() == m_modem) score += 1; // não fica pulando entre aparelhos à toa
        if (score > bestScore) { best = it.key(); bestScore = score; }
    }
    if (best != m_modem) setModem(best);
    else updateConnection();
}

void OfonoPhoneHal::setModem(const QString &path)
{
    m_modem = path;
    m_calls.clear();
    m_focus.clear();
    m_battery = 0;
    m_signal = 0;
    m_muted = false;
    if (!m_modem.isEmpty()) {
        // Pede ao oFono para conectar o viva-voz neste celular (uma vez por escolha).
        if (!powered(m_modems.value(m_modem)))
            callModem(m_modem, kModem, QStringLiteral("SetProperty"),
                      {QStringLiteral("Powered"), QVariant::fromValue(QDBusVariant(true))});
        loadModemDetails();
    }
    refresh();
    updateConnection();
}

void OfonoPhoneHal::loadModemDetails()
{
    const QStringList ifaces = interfacesOf(m_modems.value(m_modem));
    if (ifaces.contains(kVcm)) loadCalls();
    if (ifaces.contains(kVolume)) getProperties(kVolume, &OfonoPhoneHal::applyCallVolume);
    if (ifaces.contains(kNetwork)) getProperties(kNetwork, &OfonoPhoneHal::applyNetwork);
    if (ifaces.contains(kHandsfree)) getProperties(kHandsfree, &OfonoPhoneHal::applyHandsfree);
}

void OfonoPhoneHal::getProperties(const QString &interface, void (OfonoPhoneHal::*apply)(const QString &, const QVariant &))
{
    const QString modem = m_modem;
    const QDBusMessage msg = QDBusMessage::createMethodCall(kService, modem, interface, QStringLiteral("GetProperties"));
    auto *w = new QDBusPendingCallWatcher(m_system.asyncCall(msg), this);
    connect(w, &QDBusPendingCallWatcher::finished, this, [this, w, modem, apply] {
        w->deleteLater();
        const QDBusMessage r = w->reply();
        if (modem != m_modem || r.type() != QDBusMessage::ReplyMessage || r.arguments().isEmpty()) return;
        const QVariantMap props = qdbus_cast<QVariantMap>(r.arguments().at(0));
        for (auto it = props.cbegin(); it != props.cend(); ++it) (this->*apply)(it.key(), it.value());
        emit changed();
    });
}

void OfonoPhoneHal::applyCallVolume(const QString &name, const QVariant &value)
{
    if (name == u"Muted") m_muted = value.toBool();
}

void OfonoPhoneHal::applyNetwork(const QString &name, const QVariant &value)
{
    if (name == u"Strength") m_signal = qBound(0, (value.toInt() + 10) / 20, 5); // 0..100 -> 0..5
}

void OfonoPhoneHal::applyHandsfree(const QString &name, const QVariant &value)
{
    if (name == u"BatteryChargeLevel") m_battery = qBound(0, value.toInt() * 20, 100); // 0..5 -> %
}

void OfonoPhoneHal::onCallVolumeProperty(const QDBusMessage &msg)
{
    if (!fromCurrentModem(msg)) return;
    applyCallVolume(nameArg(msg), valueArg(msg));
    emit changed();
}

void OfonoPhoneHal::onNetworkProperty(const QDBusMessage &msg)
{
    if (!fromCurrentModem(msg)) return;
    applyNetwork(nameArg(msg), valueArg(msg));
    emit changed();
}

void OfonoPhoneHal::onHandsfreeProperty(const QDBusMessage &msg)
{
    if (!fromCurrentModem(msg)) return;
    applyHandsfree(nameArg(msg), valueArg(msg));
    emit changed();
}

void OfonoPhoneHal::updateConnection()
{
    const QVariantMap m = m_modems.value(m_modem);
    const bool nowConnected = !m_modem.isEmpty() && powered(m) && interfacesOf(m).contains(kVcm);
    const QString nameNow = m.value(QStringLiteral("Name")).toString();
    const QString address = addressFromPath(m_modem);
    bool dirty = nowConnected != m_connected || nameNow != m_deviceName;
    m_connected = nowConnected;
    m_deviceName = nameNow.isEmpty() ? address : nameNow;

    if (m_connected && !address.isEmpty() && address != m_syncedAddress) {
        m_syncedAddress = address;
        m_contacts.clear();
        m_phoneHistory.clear();
        m_localHistory.clear();
        refreshPhonebook();
        dirty = false; // refreshPhonebook já avisou
        refresh();
    } else if (!m_connected && m_pbap.isBusy()) {
        m_pbap.cancel();
        m_syncing = false;
        dirty = true;
    }
    if (dirty) emit changed();
}

void OfonoPhoneHal::refreshPhonebook()
{
    const QString address = addressFromPath(m_modem);
    if (!m_connected || address.isEmpty()) return;
    m_syncing = true;
    m_phonebookStatus = QStringLiteral("Baixando a agenda…");
    m_pbap.sync(address);
    emit changed();
}

// ---- Chamadas ---------------------------------------------------------------------------

void OfonoPhoneHal::loadCalls()
{
    const QString modem = m_modem;
    const QDBusMessage msg = QDBusMessage::createMethodCall(kService, modem, kVcm, QStringLiteral("GetCalls"));
    auto *w = new QDBusPendingCallWatcher(m_system.asyncCall(msg), this);
    connect(w, &QDBusPendingCallWatcher::finished, this, [this, w, modem] {
        w->deleteLater();
        const QDBusMessage r = w->reply();
        if (modem != m_modem || r.type() != QDBusMessage::ReplyMessage || r.arguments().isEmpty()) return;
        for (const OfonoObject &o : qdbus_cast<OfonoObjectList>(r.arguments().at(0)))
            updateCall(o.path.path(), o.properties);
        refresh();
    });
}

void OfonoPhoneHal::onCallManagerSignal(const QDBusMessage &msg)
{
    if (!fromCurrentModem(msg)) return;
    if (msg.member() == u"CallAdded") callAdded(pathArg(msg), mapArg(msg, 1));
    else callRemoved(pathArg(msg));
}

void OfonoPhoneHal::callAdded(const QString &path, const QVariantMap &props)
{
    updateCall(path, props);
    refresh();
}

void OfonoPhoneHal::onCallProperty(const QDBusMessage &msg)
{
    if (!fromCurrentModem(msg)) return;
    const QString path = msg.path();
    if (!m_calls.contains(path)) return;
    updateCall(path, {{nameArg(msg), valueArg(msg)}});
    refresh();
}

void OfonoPhoneHal::callRemoved(const QString &path)
{
    const auto it = m_calls.constFind(path);
    if (it == m_calls.cend()) return;
    const CallInfo c = it.value();
    m_calls.remove(path);
    const QString kind = !c.incoming ? QStringLiteral("out") : c.wasActive ? QStringLiteral("in") : QStringLiteral("missed");
    const QString display = !c.name.isEmpty() ? c.name : c.number.isEmpty() ? QStringLiteral("Número privado") : nameFor(c.number);
    m_localHistory.prepend({display, c.number, kind, QDateTime::currentDateTime()});
    refresh();
}

void OfonoPhoneHal::updateCall(const QString &path, const QVariantMap &props)
{
    CallInfo &c = m_calls[path];
    if (props.contains(QStringLiteral("State"))) {
        c.state = props.value(QStringLiteral("State")).toString();
        if (c.state == u"incoming" || c.state == u"waiting") c.incoming = true;
        if (c.state == u"active" || c.state == u"held") c.wasActive = true;
    }
    if (props.contains(QStringLiteral("LineIdentification")))
        c.number = props.value(QStringLiteral("LineIdentification")).toString();
    if (props.contains(QStringLiteral("Name")))
        c.name = props.value(QStringLiteral("Name")).toString();
}

QString OfonoPhoneHal::focusCall(const QMap<QString, CallInfo> &calls)
{
    static const QStringList priority{QStringLiteral("incoming"), QStringLiteral("waiting"), QStringLiteral("dialing"),
                                      QStringLiteral("alerting"), QStringLiteral("active"), QStringLiteral("held")};
    for (const QString &state : priority)
        for (auto it = calls.cbegin(); it != calls.cend(); ++it)
            if (it.value().state == state) return it.key();
    return QString();
}

QString OfonoPhoneHal::uiState(const QString &s)
{
    if (s == u"incoming" || s == u"waiting") return QStringLiteral("incoming");
    if (s == u"dialing" || s == u"alerting") return QStringLiteral("dialing");
    if (s == u"active" || s == u"held") return QStringLiteral("active");
    return QStringLiteral("idle");
}

QString OfonoPhoneHal::addressFromPath(const QString &modemPath)
{
    static const QRegularExpression re(QStringLiteral("dev_([0-9A-Fa-f]{2}(?:_[0-9A-Fa-f]{2}){5})"));
    const QRegularExpressionMatch m = re.match(modemPath);
    return m.hasMatch() ? m.captured(1).replace(u'_', u':').toUpper() : QString();
}

/** Recalcula o que a tela mostra a partir das chamadas do oFono. */
void OfonoPhoneHal::refresh()
{
    const QString focus = focusCall(m_calls);
    const CallInfo c = m_calls.value(focus);
    const QString state = uiState(c.state);
    if (focus != m_focus || (state == u"active" && m_callState != u"active")) {
        m_callSeconds = 0; // chamada nova, ou acabou de ser atendida
    }
    m_focus = focus;
    m_callState = state;
    m_callNumber = c.number;
    m_callName = !c.name.isEmpty() ? c.name : c.number.isEmpty() ? (focus.isEmpty() ? QString() : QStringLiteral("Número privado"))
                                                                  : nameFor(c.number);
    if (state == u"active") { if (!m_tick.isActive()) m_tick.start(); }
    else m_tick.stop();
    if (state == u"idle") m_muted = false;

    m_recents = m_localHistory + m_phoneHistory;
    if (m_recents.size() > kMaxRecents) m_recents.resize(kMaxRecents);
    emit changed();
}

void OfonoPhoneHal::callModem(const QString &path, const QString &interface, const QString &method, const QVariantList &args)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(kService, path, interface, method);
    msg.setArguments(args);
    auto *w = new QDBusPendingCallWatcher(m_system.asyncCall(msg), this);
    connect(w, &QDBusPendingCallWatcher::finished, this, [w, method] {
        w->deleteLater();
        const QDBusMessage r = w->reply();
        if (r.type() == QDBusMessage::ErrorMessage)
            qWarning().noquote() << "oFono" << method << "falhou:" << r.errorName() << r.errorMessage();
    });
}

void OfonoPhoneHal::dial(const QString &number)
{
    const QString n = number.trimmed();
    if (!m_connected || inCall() || n.isEmpty()) return;
    callModem(m_modem, kVcm, QStringLiteral("Dial"), {n, QStringLiteral("default")});
}

void OfonoPhoneHal::accept()
{
    const QString s = m_calls.value(m_focus).state;
    if (s == u"incoming") callModem(m_focus, kCall, QStringLiteral("Answer"));
    else if (s == u"waiting") callModem(m_modem, kVcm, QStringLiteral("HoldAndAnswer")); // põe a atual em espera
}

void OfonoPhoneHal::reject()
{
    if (m_callState == u"incoming") callModem(m_focus, kCall, QStringLiteral("Hangup"));
}

void OfonoPhoneHal::hangup()
{
    if (!m_focus.isEmpty()) callModem(m_focus, kCall, QStringLiteral("Hangup"));
}

void OfonoPhoneHal::setMuted(bool muted)
{
    if (!inCall() || muted == m_muted) return;
    callModem(m_modem, kVolume, QStringLiteral("SetProperty"), {QStringLiteral("Muted"), QVariant::fromValue(QDBusVariant(muted))});
    m_muted = muted; // o PropertyChanged confirma (ou desfaz)
    emit changed();
}

void OfonoPhoneHal::sendDtmf(const QString &digit)
{
    if (m_callState == u"active" && !digit.isEmpty()) callModem(m_modem, kVcm, QStringLiteral("SendTones"), {digit});
}
