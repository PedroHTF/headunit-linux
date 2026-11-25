#include "hal/SimulatedPhoneHal.h"

SimulatedPhoneHal::SimulatedPhoneHal(QObject *parent, int answerDelayMs) : PhoneHal(parent)
{
    m_contacts = {
        {"Ana Souza", "+55 11 91234-5678"},   {"Bruno Lima", "+55 11 98765-4321"},
        {"Carla Mendes", "+55 21 99876-1234"}, {"Casa", "+55 11 3456-7890"},
        {"Diego Rocha", "+55 31 99123-4567"},  {"Escritório", "+55 11 2345-6789"},
        {"Fernanda Alves", "+55 11 97654-3210"}, {"Oficina", "+55 11 4002-8922"},
    };
    const QDateTime now = QDateTime::currentDateTime();
    m_recents = {
        {"Ana Souza", "+55 11 91234-5678", "in", now.addSecs(-3600)},
        {"Oficina", "+55 11 4002-8922", "out", now.addSecs(-3 * 3600)},
        {"+55 11 95555-0000", "+55 11 95555-0000", "missed", now.addDays(-1)},
    };
    m_connected = true;
    m_deviceName = QStringLiteral("Celular (simulado)");
    m_battery = 78;
    m_signal = 4;

    m_answer.setSingleShot(true);
    m_answer.setInterval(answerDelayMs);
    connect(&m_answer, &QTimer::timeout, this, [this] {
        if (m_callState != u"dialing") return;
        m_callState = QStringLiteral("active");
        m_tick.start();
        emit changed();
    });
    m_tick.setInterval(1000);
    connect(&m_tick, &QTimer::timeout, this, [this] {
        ++m_callSeconds;
        emit changed();
    });
}

void SimulatedPhoneHal::startCall(const QString &number, const QString &state)
{
    m_callNumber = number;
    m_callName = nameFor(number);
    m_callState = state;
    m_callSeconds = 0;
    m_muted = false;
    emit changed();
}

void SimulatedPhoneHal::dial(const QString &number)
{
    if (!m_connected || inCall() || number.trimmed().isEmpty()) return;
    m_direction = QStringLiteral("out");
    startCall(number.trimmed(), QStringLiteral("dialing"));
    m_answer.start();
}

void SimulatedPhoneHal::simulateIncoming(const QString &number)
{
    if (!m_connected || inCall()) return;
    m_direction = QStringLiteral("in");
    startCall(number, QStringLiteral("incoming"));
}

void SimulatedPhoneHal::accept()
{
    if (m_callState != u"incoming") return;
    m_callState = QStringLiteral("active");
    m_tick.start();
    emit changed();
}

void SimulatedPhoneHal::reject()
{
    if (m_callState == u"incoming") endCall(QStringLiteral("missed"));
}

void SimulatedPhoneHal::hangup()
{
    if (m_callState == u"incoming") endCall(QStringLiteral("missed"));
    else if (inCall()) endCall(m_direction);
}

void SimulatedPhoneHal::simulateRemoteHangup() { hangup(); }

void SimulatedPhoneHal::endCall(const QString &kind)
{
    m_answer.stop();
    m_tick.stop();
    m_recents.prepend({m_callName, m_callNumber, kind, QDateTime::currentDateTime()});
    if (m_recents.size() > 30) m_recents.resize(30);
    m_callState = QStringLiteral("idle");
    m_callName.clear();
    m_callNumber.clear();
    m_callSeconds = 0;
    m_muted = false;
    emit changed();
}

void SimulatedPhoneHal::setMuted(bool muted)
{
    if (!inCall() || muted == m_muted) return;
    m_muted = muted;
    emit changed();
}

void SimulatedPhoneHal::setConnected(bool connected)
{
    if (connected == m_connected) return;
    if (!connected && inCall()) endCall(m_direction);
    m_connected = connected;
    emit changed();
}
