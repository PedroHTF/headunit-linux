#pragma once

#include "projection/DongleTransport.h"

#include <QElapsedTimer>
#include <QPointF>
#include <QTimer>

/**
 * Dongle de mentira: responde à inicialização, "conecta" um celular (CarPlay ou Android
 * Auto), desenha uma tela parecida com a do celular e reage ao toque. Para testar a
 * tela Projeção, o toque e os botões no PC.
 */
class SimulatedDongleTransport : public DongleTransport
{
    Q_OBJECT
public:
    explicit SimulatedDongleTransport(QObject *parent = nullptr, int connectDelayMs = 1200);

    bool isSimulated() const override { return true; }
    void start() override;
    void stop() override;
    void write(const QByteArray &message) override;

    /** Simulador: liga um celular (CarPlay = iPhone, Android Auto = Android) ou desliga. */
    void plugPhone(dongle::PhoneType type);
    /** Conectar um celular sozinho logo após a inicialização (padrão: não). */
    void setAutoPlug(bool v) { m_autoPlug = v; }
    void unplugPhone();

    /** Mensagens recebidas da central (para testes). */
    QList<QByteArray> written() const { return m_written; }
    QImage render() const;

private:
    void onCommand(dongle::Command c);

    QList<QByteArray> m_written;
    QTimer m_connect;
    QTimer m_frames;
    bool m_running = false;
    bool m_plugged = false;
    bool m_autoPlug = false;
    dongle::PhoneType m_phone = dongle::PhoneType::CarPlay;
    int m_width = 800, m_height = 480;
    QPointF m_touch{-1, -1};
    QString m_openApp;
    QElapsedTimer m_since;
};
