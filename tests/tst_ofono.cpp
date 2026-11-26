// Testa o OfonoPhoneHal e o PbapClient contra um oFono e um obexd FALSOS, publicados num
// D-Bus de sessão próprio (o ctest roda este teste dentro de dbus-run-session).
#include "hal/OfonoPhoneHal.h"

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QTemporaryDir>
#include <QtTest>

namespace {
const QString kModemPath = QStringLiteral("/hfp/org/bluez/hci0/dev_AA_BB_CC_DD_EE_01");
const QString kNumberBruno = QStringLiteral("+55 11 98765-4321");
}

// ---- oFono falso --------------------------------------------------------------------------

class ManagerAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ofono.Manager")
public:
    explicit ManagerAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent) {}
    OfonoObjectList modems;
public slots:
    OfonoObjectList GetModems() { return modems; }
signals:
    void ModemAdded(const QDBusObjectPath &path, const QVariantMap &props);
    void ModemRemoved(const QDBusObjectPath &path);
};

// Interfaces com GetProperties/SetProperty/PropertyChanged, como todas do oFono.
class ModemAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ofono.Modem")
public:
    explicit ModemAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent) {}
    QVariantMap props;
    void set(const QString &name, const QVariant &v)
    {
        props[name] = v;
        emit PropertyChanged(name, QDBusVariant(v));
    }
public slots:
    QVariantMap GetProperties() { return props; }
    void SetProperty(const QString &name, const QDBusVariant &v) { set(name, v.variant()); }
signals:
    void PropertyChanged(const QString &name, const QDBusVariant &value);
};

class VolumeAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ofono.CallVolume")
public:
    explicit VolumeAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent) {}
    QVariantMap props;
    void set(const QString &name, const QVariant &v)
    {
        props[name] = v;
        emit PropertyChanged(name, QDBusVariant(v));
    }
public slots:
    QVariantMap GetProperties() { return props; }
    void SetProperty(const QString &name, const QDBusVariant &v) { set(name, v.variant()); }
signals:
    void PropertyChanged(const QString &name, const QDBusVariant &value);
};

class NetworkAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ofono.NetworkRegistration")
public:
    explicit NetworkAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent) {}
    QVariantMap props;
    void set(const QString &name, const QVariant &v)
    {
        props[name] = v;
        emit PropertyChanged(name, QDBusVariant(v));
    }
public slots:
    QVariantMap GetProperties() { return props; }
    void SetProperty(const QString &name, const QDBusVariant &v) { set(name, v.variant()); }
signals:
    void PropertyChanged(const QString &name, const QDBusVariant &value);
};

class HandsfreeAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ofono.Handsfree")
public:
    explicit HandsfreeAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent) {}
    QVariantMap props;
    void set(const QString &name, const QVariant &v)
    {
        props[name] = v;
        emit PropertyChanged(name, QDBusVariant(v));
    }
public slots:
    QVariantMap GetProperties() { return props; }
    void SetProperty(const QString &name, const QDBusVariant &v) { set(name, v.variant()); }
signals:
    void PropertyChanged(const QString &name, const QDBusVariant &value);
};


class FakeCall;

class VcmAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ofono.VoiceCallManager")
public:
    explicit VcmAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent) {}
    std::function<QDBusObjectPath(const QString &)> onDial;
    QStringList tones;
    int holdAndAnswer = 0;
public slots:
    QDBusObjectPath Dial(const QString &number, const QString &) { return onDial(number); }
    OfonoObjectList GetCalls() { return {}; }
    void HoldAndAnswer() { ++holdAndAnswer; }
    void SendTones(const QString &t) { tones << t; }
signals:
    void CallAdded(const QDBusObjectPath &path, const QVariantMap &props);
    void CallRemoved(const QDBusObjectPath &path);
};

class CallAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ofono.VoiceCall")
public:
    explicit CallAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent) {}
    std::function<void()> onAnswer, onHangup;
public slots:
    void Answer() { onAnswer(); }
    void Hangup() { onHangup(); }
signals:
    void PropertyChanged(const QString &name, const QDBusVariant &value);
};

class FakeOfono : public QObject
{
    Q_OBJECT
public:
    explicit FakeOfono(QDBusConnection bus) : m_bus(bus)
    {
        manager = new ManagerAdaptor(&m_root);
        modem = new ModemAdaptor(&m_modem);
        vcm = new VcmAdaptor(&m_modem);
        volume = new VolumeAdaptor(&m_modem);
        network = new NetworkAdaptor(&m_modem);
        handsfree = new HandsfreeAdaptor(&m_modem);
        modem->props = {{"Powered", true}, {"Online", true}, {"Name", "Pixel do Pedro"}, {"Type", "hfp"},
                        {"Interfaces", QStringList{"org.ofono.VoiceCallManager", "org.ofono.CallVolume",
                                                   "org.ofono.NetworkRegistration", "org.ofono.Handsfree"}}};
        network->props = {{"Strength", QVariant::fromValue<uchar>(60)}};
        handsfree->props = {{"BatteryChargeLevel", QVariant::fromValue<uchar>(4)}};
        volume->props = {{"Muted", false}};
        manager->modems = {{QDBusObjectPath(kModemPath), modem->props}};
        vcm->onDial = [this](const QString &number) { return addCall(number, "dialing"); };
        QVERIFY(m_bus.registerObject("/", &m_root));
        QVERIFY(m_bus.registerObject(kModemPath, &m_modem));
        QVERIFY(m_bus.registerService("org.ofono"));
    }

    QDBusObjectPath addCall(const QString &number, const QString &state)
    {
        const QString path = kModemPath + QStringLiteral("/voicecall%1").arg(++m_next, 2, 10, QChar('0'));
        auto *obj = new QObject(this);
        auto *call = new CallAdaptor(obj);
        calls.insert(path, call);
        m_bus.registerObject(path, obj);
        call->onAnswer = [this, path] { setCallState(path, "active"); };
        call->onHangup = [this, path] { removeCall(path); };
        // Como o oFono: a resposta do Dial sai antes do CallAdded.
        QTimer::singleShot(0, this, [this, path, number, state] {
            emit vcm->CallAdded(QDBusObjectPath(path), {{"State", state}, {"LineIdentification", number}, {"Name", ""}});
        });
        return QDBusObjectPath(path);
    }
    void setCallState(const QString &path, const QString &state)
    {
        emit calls.value(path)->PropertyChanged("State", QDBusVariant(state));
    }
    void removeCall(const QString &path)
    {
        if (!calls.contains(path)) return;
        setCallState(path, "disconnected");
        emit vcm->CallRemoved(QDBusObjectPath(path));
        m_bus.unregisterObject(path);
        calls.remove(path);
    }

    ManagerAdaptor *manager;
    ModemAdaptor *modem;
    VcmAdaptor *vcm;
    VolumeAdaptor *volume;
    NetworkAdaptor *network;
    HandsfreeAdaptor *handsfree;
    QMap<QString, CallAdaptor *> calls;

private:
    QDBusConnection m_bus;
    QObject m_root, m_modem;
    int m_next = 0;
};

// ---- obexd falso (PBAP) --------------------------------------------------------------------

class ObexClientAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Client1")
public:
    explicit ObexClientAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent) {}
    QStringList destinations;
    int removed = 0;
public slots:
    QDBusObjectPath CreateSession(const QString &dest, const QVariantMap &opts)
    {
        destinations << dest + u'/' + opts.value("Target").toString();
        return QDBusObjectPath("/org/bluez/obex/client/session0");
    }
    void RemoveSession(const QDBusObjectPath &) { ++removed; }
};

class PbapAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.PhonebookAccess1")
public:
    explicit PbapAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent) {}
    std::function<QDBusObjectPath(const QString &book, QVariantMap &props)> onPull;
    QString selected;
public slots:
    void Select(const QString &location, const QString &book) { selected = location + u'/' + book; }
    QDBusObjectPath PullAll(const QString &, const QVariantMap &, QVariantMap &props) { return onPull(selected, props); }
};

class FakeObex : public QObject
{
    Q_OBJECT
public:
    FakeObex(QDBusConnection bus, const QString &dir) : m_bus(bus), m_dir(dir)
    {
        client = new ObexClientAdaptor(&m_client);
        pbap = new PbapAdaptor(&m_session);
        pbap->onPull = [this](const QString &book, QVariantMap &props) {
            const QString path = QStringLiteral("/org/bluez/obex/client/session0/transfer%1").arg(++m_next);
            const QString file = m_dir + QStringLiteral("/vcard%1").arg(m_next);
            QFile f(file);
            f.open(QIODevice::WriteOnly);
            f.write(book.endsWith("pb") ? phonebook : history);
            f.close();
            props = {{"Filename", file}, {"Status", "queued"}};
            QTimer::singleShot(20, this, [this, path] {
                QDBusMessage sig = QDBusMessage::createSignal(path, "org.freedesktop.DBus.Properties", "PropertiesChanged");
                sig << QStringLiteral("org.bluez.obex.Transfer1") << QVariantMap{{"Status", "complete"}} << QStringList();
                m_bus.send(sig);
            });
            return QDBusObjectPath(path);
        };
        QVERIFY(m_bus.registerObject("/org/bluez/obex", &m_client));
        QVERIFY(m_bus.registerObject("/org/bluez/obex/client/session0", &m_session));
        QVERIFY(m_bus.registerService("org.bluez.obex"));
    }

    QByteArray phonebook =
        "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Bruno Lima\r\nTEL;TYPE=CELL:+55 11 98765-4321\r\nEND:VCARD\r\n"
        "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Ana Souza\r\nTEL:+55 11 91234-5678\r\nEND:VCARD\r\n";
    QByteArray history =
        "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Ana Souza\r\nTEL:+55 11 91234-5678\r\n"
        "X-IRMC-CALL-DATETIME;TYPE=DIALED:20261125T081500\r\nEND:VCARD\r\n";
    ObexClientAdaptor *client;
    PbapAdaptor *pbap;

private:
    QDBusConnection m_bus;
    QString m_dir;
    QObject m_client, m_session;
    int m_next = 0;
};

// ---- Testes ---------------------------------------------------------------------------------

class TestOfono : public QObject
{
    Q_OBJECT
private:
    QDBusConnection fakeBus() { return QDBusConnection::connectToBus(QDBusConnection::SessionBus, QStringLiteral("fake")); }

private slots:
    void initTestCase()
    {
        if (!QDBusConnection::sessionBus().isConnected())
            QSKIP("Sem D-Bus de sessão (rode com dbus-run-session)");
        registerOfonoTypes();
    }

    void pureHelpers()
    {
        QCOMPARE(OfonoPhoneHal::addressFromPath(kModemPath), QStringLiteral("AA:BB:CC:DD:EE:01"));
        QCOMPARE(OfonoPhoneHal::addressFromPath("/ril_0"), QString());
        QMap<QString, OfonoPhoneHal::CallInfo> calls;
        calls["/a"].state = "active";
        calls["/b"].state = "waiting";
        QCOMPARE(OfonoPhoneHal::focusCall(calls), QStringLiteral("/b")); // a que chega aparece por cima
        QCOMPARE(OfonoPhoneHal::uiState("alerting"), QStringLiteral("dialing"));
        QCOMPARE(OfonoPhoneHal::uiState("held"), QStringLiteral("active"));
        QCOMPARE(OfonoPhoneHal::focusCall({}), QString());
    }

    void fullFlow()
    {
        QTemporaryDir dir;
        QDBusConnection bus = fakeBus();
        FakeOfono ofono(bus);
        FakeObex obex(bus, dir.path());
        QVERIFY(OfonoPhoneHal::isAvailable(QDBusConnection::sessionBus()));

        OfonoPhoneHal phone(QDBusConnection::sessionBus(), QDBusConnection::sessionBus());
        QVERIFY(!phone.isSimulated());

        // Conexão, bateria e sinal
        QTRY_VERIFY(phone.isConnected());
        QCOMPARE(phone.deviceName(), QStringLiteral("Pixel do Pedro"));
        QTRY_COMPARE(phone.battery(), 80);
        QTRY_COMPARE(phone.signal(), 3);

        // Agenda e histórico pelo PBAP, no endereço do celular
        QTRY_VERIFY(!phone.isSyncing());
        QCOMPARE(obex.client->destinations, QStringList{"AA:BB:CC:DD:EE:01/PBAP"});
        QCOMPARE(phone.contacts().size(), 2);
        QCOMPARE(phone.contacts().first().name, QStringLiteral("Ana Souza"));
        QCOMPARE(phone.recents().size(), 1);
        QCOMPARE(phone.recents().first().kind, QStringLiteral("out"));
        QCOMPARE(phone.phonebookStatus(), QStringLiteral("2 contatos"));
        QTRY_COMPARE(obex.client->removed, 1); // sessão fechada
        QCOMPARE(QDir(dir.path()).entryList(QDir::Files).size(), 0); // temporários apagados

        // Chamada recebida: nome vem da agenda; atender; desligar
        const QString in = ofono.addCall(kNumberBruno, "incoming").path();
        QTRY_COMPARE(phone.callState(), QStringLiteral("incoming"));
        QCOMPARE(phone.callName(), QStringLiteral("Bruno Lima"));
        phone.accept();
        QTRY_COMPARE(phone.callState(), QStringLiteral("active"));
        QTRY_VERIFY(phone.callSeconds() >= 1);
        phone.sendDtmf("5");
        QTRY_COMPARE(ofono.vcm->tones, QStringList{"5"});
        phone.setMuted(true);
        QTRY_COMPARE(ofono.volume->props.value("Muted").toBool(), true);
        phone.hangup();
        QTRY_COMPARE(phone.callState(), QStringLiteral("idle"));
        QVERIFY(!ofono.calls.contains(in));
        QCOMPARE(phone.recents().first().kind, QStringLiteral("in"));
        QVERIFY(!phone.isMuted());

        // Chamada que ninguém atende: perdida
        const QString missed = ofono.addCall("+55 11 95555-0000", "incoming").path();
        QTRY_COMPARE(phone.callState(), QStringLiteral("incoming"));
        QCOMPARE(phone.callName(), QStringLiteral("+55 11 95555-0000"));
        ofono.removeCall(missed);
        QTRY_COMPARE(phone.callState(), QStringLiteral("idle"));
        QCOMPARE(phone.recents().first().kind, QStringLiteral("missed"));

        // Ligar: discando -> chamando -> atendida
        phone.dial("+55 11 91234-5678");
        QTRY_COMPARE(phone.callState(), QStringLiteral("dialing"));
        QCOMPARE(phone.callName(), QStringLiteral("Ana Souza"));
        const QString out = ofono.calls.lastKey();
        ofono.setCallState(out, "alerting");
        QTest::qWait(50);
        QCOMPARE(phone.callState(), QStringLiteral("dialing"));
        ofono.setCallState(out, "active");
        QTRY_COMPARE(phone.callState(), QStringLiteral("active"));

        // Chamada em espera chegando durante a ativa: aparece; atender usa HoldAndAnswer
        ofono.addCall(QStringLiteral("+55 31 99123-4567"), "waiting");
        QTRY_COMPARE(phone.callState(), QStringLiteral("incoming"));
        phone.accept();
        QTRY_COMPARE(ofono.vcm->holdAndAnswer, 1);

        // Celular desconecta (Powered=false): some a conexão, a sincronização não roda de novo
        ofono.modem->set("Powered", false);
        QTRY_VERIFY(!phone.isConnected());
    }

    void reconnectAndRefresh()
    {
        QTemporaryDir dir;
        QDBusConnection bus = fakeBus();
        FakeOfono ofono(bus);
        ofono.modem->props["Powered"] = false;
        ofono.manager->modems = {{QDBusObjectPath(kModemPath), ofono.modem->props}};
        FakeObex obex(bus, dir.path());

        OfonoPhoneHal phone(QDBusConnection::sessionBus(), QDBusConnection::sessionBus());
        // Ao escolher o celular desligado, a central pede Powered=true (conectar o viva-voz)
        QTRY_VERIFY(phone.isConnected());
        QTRY_VERIFY(!phone.isSyncing());
        QCOMPARE(phone.contacts().size(), 2);

        obex.phonebook += "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Casa\r\nTEL:3456-7890\r\nEND:VCARD\r\n";
        phone.refreshPhonebook();
        QVERIFY(phone.isSyncing());
        QTRY_VERIFY(!phone.isSyncing());
        QCOMPARE(phone.contacts().size(), 3);
        QCOMPARE(obex.client->destinations.size(), 2);
    }

    /**
     * Só para capturas de tela: com HEADUNIT_FAKE_OFONO_SECONDS=N, publica os falsos por N s
     * para o app rodar com --phone ofono (DBUS_SYSTEM_BUS_ADDRESS apontando para este barramento).
     */
    void serveFakesForScreenshots()
    {
        const int seconds = qEnvironmentVariableIntValue("HEADUNIT_FAKE_OFONO_SECONDS");
        if (seconds <= 0) QSKIP("Só com HEADUNIT_FAKE_OFONO_SECONDS");
        QTemporaryDir dir;
        QDBusConnection bus = fakeBus();
        FakeOfono ofono(bus);
        FakeObex obex(bus, dir.path());
        obex.phonebook +=
            "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Carla Mendes\r\nTEL:+55 21 99876-1234\r\nEND:VCARD\r\n"
            "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Oficina\r\nTEL:+55 11 4002-8922\r\nEND:VCARD\r\n";
        if (qEnvironmentVariableIsSet("HEADUNIT_FAKE_OFONO_CALL"))
            QTimer::singleShot(800, &ofono, [&ofono] { ofono.addCall(kNumberBruno, "incoming"); });
        QTest::qWait(seconds * 1000);
    }

    void cleanup()
    {
        QDBusConnection bus = fakeBus();
        bus.unregisterService("org.ofono");
        bus.unregisterService("org.bluez.obex");
        QDBusConnection::disconnectFromBus(QStringLiteral("fake"));
    }
};

QTEST_GUILESS_MAIN(TestOfono)
#include "tst_ofono.moc"
