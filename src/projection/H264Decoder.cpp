#include "projection/H264Decoder.h"

#ifdef HEADUNIT_HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

struct H264Decoder::Impl
{
    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    AVCodecContext *ctx = nullptr;
    AVCodecParserContext *parser = nullptr;
    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    SwsContext *sws = nullptr;

    Impl()
    {
        if (!codec) return;
        ctx = avcodec_alloc_context3(codec);
        ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
        if (avcodec_open2(ctx, codec, nullptr) < 0) {
            avcodec_free_context(&ctx);
            return;
        }
        parser = av_parser_init(AV_CODEC_ID_H264);
    }
    ~Impl()
    {
        if (sws) sws_freeContext(sws);
        if (parser) av_parser_close(parser);
        if (ctx) avcodec_free_context(&ctx);
        av_frame_free(&frame);
        av_packet_free(&pkt);
    }

    QImage toImage()
    {
        sws = sws_getCachedContext(sws, frame->width, frame->height, AVPixelFormat(frame->format),
                                   frame->width, frame->height, AV_PIX_FMT_BGRA, SWS_BILINEAR,
                                   nullptr, nullptr, nullptr);
        if (!sws) return {};
        QImage img(frame->width, frame->height, QImage::Format_RGB32);
        uint8_t *dst[1] = {img.bits()};
        int stride[1] = {int(img.bytesPerLine())};
        sws_scale(sws, frame->data, frame->linesize, 0, frame->height, dst, stride);
        return img;
    }
};

H264Decoder::H264Decoder() : d(std::make_unique<Impl>()) {}
H264Decoder::~H264Decoder() = default;
bool H264Decoder::available() { return avcodec_find_decoder(AV_CODEC_ID_H264) != nullptr; }

QImage H264Decoder::decode(const QByteArray &h264)
{
    if (!d->ctx || !d->parser) return {};
    QImage last;
    const uint8_t *data = reinterpret_cast<const uint8_t *>(h264.constData());
    int size = int(h264.size());
    while (size > 0) {
        const int used = av_parser_parse2(d->parser, d->ctx, &d->pkt->data, &d->pkt->size, data, size,
                                          AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (used < 0) break;
        data += used;
        size -= used;
        if (d->pkt->size == 0) continue;
        if (avcodec_send_packet(d->ctx, d->pkt) < 0) continue;
        while (avcodec_receive_frame(d->ctx, d->frame) == 0) last = d->toImage();
    }
    return last;
}

#else // sem FFmpeg

struct H264Decoder::Impl {};
H264Decoder::H264Decoder() : d(std::make_unique<Impl>()) {}
H264Decoder::~H264Decoder() = default;
bool H264Decoder::available() { return false; }
QImage H264Decoder::decode(const QByteArray &) { return {}; }

#endif
