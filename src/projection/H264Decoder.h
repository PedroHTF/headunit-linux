#pragma once

#include <QByteArray>
#include <QImage>
#include <memory>

/**
 * Decodifica o vídeo H.264 que o dongle manda (FFmpeg/libavcodec). Sem FFmpeg na
 * compilação, [available] é falso e nada é decodificado.
 */
class H264Decoder
{
public:
    H264Decoder();
    ~H264Decoder();
    H264Decoder(const H264Decoder &) = delete;
    H264Decoder &operator=(const H264Decoder &) = delete;

    static bool available();
    /** Entrega um pedaço do fluxo; devolve o último quadro completo (ou nulo). */
    QImage decode(const QByteArray &h264);

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};
