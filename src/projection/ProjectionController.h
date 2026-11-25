#pragma once

#include "projection/DongleTransport.h"
#include "projection/H264Decoder.h"

#include <QAudioSink>
#include <QElapsedTimer>
#include <QHash>
#include <QImage>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <memory>
#include <unordered_map>

/**
 * CarPlay / Android Auto pelo dongle USB: inicializa o dongle, mantém o heartbeat,
 * decodifica vídeo, toca o áudio e repassa toques e botões.
 *
 * Estados: "searching" (sem dongle), "ready" (dongle pronto, sem celular),
 * "connected" (celular projetando).
 */
class ProjectionController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString phoneType READ phoneType NOTIFY stateChanged)
    Q_PROPERTY(bool simulated READ isSimulated NOTIFY stateChanged)
    Q_PROPERTY(bool decoderAvailable READ decoderAvailable CONSTANT)
    Q_PROPERTY(bool usbAvailable READ usbAvailable CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY stateChanged)
    Q_PROPERTY(QString mediaTitle READ mediaTitle NOTIFY mediaChanged)
    Q_PROPERTY(QString mediaArtist READ mediaArtist NOTIFY mediaChanged)
    Q_PROPERTY(int fps READ fps NOTIFY frameChanged)

public:
    explicit ProjectionController(bool simulated, QObject *parent = nullptr);
    ~ProjectionController() override;

    QString state() const { return m_state; }
    QString phoneType() const { return m_phoneType; }
    bool isConnected() const { return m_state == u"connected"; }
    bool isSimulated() const;
    static bool decoderAvailable() { return H264Decoder::available(); }
    static bool usbAvailable();
    QString lastError() const { return m_error; }
    QString mediaTitle() const { return m_mediaTitle; }
    QString mediaArtist() const { return m_mediaArtist; }
    int fps() const { return m_fps; }
    QImage frame() const { return m_frame; }

    /** Tamanho do vídeo pedido ao celular (área da tela Projeção). */
    void setVideoSize(int w, int h);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    /** Troca entre dongle real e simulado. */
    Q_INVOKABLE void setSimulated(bool simulated);
    /** action: "down", "move" ou "up"; x/y de 0 a 1. */
    Q_INVOKABLE void touch(const QString &action, double x, double y);
    /** "home", "back", "siri", "playpause", "next", "prev", "accept", "reject". */
    Q_INVOKABLE void key(const QString &name);
    /** Simulador: conecta um celular ("carplay"/"androidauto") ou desliga (""). */
    Q_INVOKABLE void simulatePhone(const QString &type);

    DongleTransport *transport() const { return m_transport; }

signals:
    void stateChanged();
    void frameChanged();
    void mediaChanged();
    /** O celular começou a tocar mídia: o app deve pausar o rádio/pendrive. */
    void mediaAudioStarted();

private:
    void createTransport(bool simulated);
    void setState(const QString &s);
    void onOpened();
    void onMessage(dongle::Type type, const QByteArray &payload);
    void setFrame(const QImage &img);
    void playAudio(const dongle::AudioPacket &a);

    QPointer<DongleTransport> m_transport;
    dongle::Config m_config;
    H264Decoder m_decoder;
    QTimer m_heartbeat;
    QString m_state = QStringLiteral("searching");
    QString m_phoneType;
    QString m_error;
    QString m_mediaTitle, m_mediaArtist;
    QImage m_frame;
    int m_frames = 0, m_fps = 0;
    QTimer m_fpsTimer;
    bool m_started = false;
    // Uma saída de áudio por tipo de fluxo (mídia, navegação, ligação...).
    struct Sink { std::unique_ptr<QAudioSink> sink; QIODevice *io = nullptr; uint32_t decodeType = 0; };
    std::unordered_map<uint32_t, Sink> m_sinks;
};
