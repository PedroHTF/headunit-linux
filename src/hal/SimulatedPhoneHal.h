#pragma once

#include "hal/PhoneHal.h"

#include <QTimer>

/**
 * Celular de mentira: agenda fictícia, chamadas que "atendem" depois de [answerDelayMs]
 * e chamadas recebidas disparadas pelo simulador. Não produz áudio.
 */
class SimulatedPhoneHal : public PhoneHal
{
    Q_OBJECT
public:
    explicit SimulatedPhoneHal(QObject *parent = nullptr, int answerDelayMs = 2500);

    QString name() const override { return QStringLiteral("Telefone simulado"); }
    bool isSimulated() const override { return true; }

    void dial(const QString &number) override;
    void accept() override;
    void reject() override;
    void hangup() override;
    void setMuted(bool muted) override;
    void sendDtmf(const QString &) override {}

    /** Simulador: um celular conecta/desconecta. */
    Q_INVOKABLE void setConnected(bool connected);
    /** Simulador: chega uma ligação. */
    Q_INVOKABLE void simulateIncoming(const QString &number);
    /** Simulador: o outro lado desliga. */
    Q_INVOKABLE void simulateRemoteHangup();

private:
    void startCall(const QString &number, const QString &state);
    void endCall(const QString &kind);

    QTimer m_answer;
    QTimer m_tick;
    QString m_direction; // "in" ou "out"
};
