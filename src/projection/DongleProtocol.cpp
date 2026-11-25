#include "projection/DongleProtocol.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace dongle {

namespace {
void putU32(QByteArray &b, uint32_t v)
{
    char buf[4];
    qToLittleEndian(v, buf);
    b.append(buf, 4);
}

uint32_t getU32(const QByteArray &b, int off) { return qFromLittleEndian<uint32_t>(b.constData() + off); }

float getF32(const QByteArray &b, int off)
{
    const uint32_t raw = getU32(b, off);
    float f;
    std::memcpy(&f, &raw, 4);
    return f;
}

void putF32(QByteArray &b, float f)
{
    uint32_t raw;
    std::memcpy(&raw, &f, 4);
    putU32(b, raw);
}
} // namespace

QByteArray encode(Type type, const QByteArray &payload)
{
    QByteArray out;
    out.reserve(HeaderSize + payload.size());
    const uint32_t t = static_cast<uint32_t>(type);
    putU32(out, Magic);
    putU32(out, uint32_t(payload.size()));
    putU32(out, t);
    putU32(out, ~t);
    out.append(payload);
    return out;
}

QByteArray open(const Config &c)
{
    QByteArray p;
    for (uint32_t v : {uint32_t(c.width), uint32_t(c.height), uint32_t(c.fps), 5u /*format*/,
                       49152u /*packetMax*/, 2u /*iBoxVersion*/, 2u /*phoneWorkMode*/})
        putU32(p, v);
    return encode(Type::Open, p);
}

QByteArray touch(TouchAction action, double x, double y)
{
    auto scale = [](double v) { return uint32_t(std::clamp(10000.0 * v, 0.0, 10000.0)); };
    QByteArray p;
    putU32(p, uint32_t(action));
    putU32(p, scale(x));
    putU32(p, scale(y));
    putU32(p, 0);
    return encode(Type::Touch, p);
}

QByteArray command(Command c)
{
    QByteArray p;
    putU32(p, uint32_t(c));
    return encode(Type::Command, p);
}

QByteArray heartbeat() { return encode(Type::HeartBeat); }

QByteArray sendFile(const QByteArray &path, const QByteArray &content)
{
    QByteArray p;
    putU32(p, uint32_t(path.size() + 1)); // inclui o \0
    p.append(path);
    p.append('\0');
    putU32(p, uint32_t(content.size()));
    p.append(content);
    return encode(Type::SendFile, p);
}

QByteArray sendNumber(const QByteArray &path, uint32_t value)
{
    QByteArray c;
    putU32(c, value);
    return sendFile(path, c);
}

QByteArray sendString(const QByteArray &path, const QByteArray &value) { return sendFile(path, value); }

QByteArray boxSettings(int width, int height, qint64 nowMs, int mediaDelayMs)
{
    const QJsonObject o{{"mediaDelay", mediaDelayMs}, {"syncTime", double(nowMs)},
                        {"androidAutoSizeW", width}, {"androidAutoSizeH", height}};
    return encode(Type::BoxSettings, QJsonDocument(o).toJson(QJsonDocument::Compact));
}

QList<QByteArray> startupSequence(const Config &c, qint64 nowMs)
{
    return {
        sendNumber("/tmp/screen_dpi", uint32_t(c.dpi)),
        open(c),
        sendNumber("/tmp/night_mode", c.nightMode ? 1 : 0),
        sendNumber("/tmp/hand_drive_mode", c.rightHandDrive ? 1 : 0),
        sendNumber("/tmp/charge_mode", 1),
        sendString("/etc/box_name", c.boxName.toLatin1()),
        boxSettings(c.width, c.height, nowMs),
        command(Command::WifiEnable),
        command(c.wifi5g ? Command::Wifi5g : Command::Wifi24g),
        command(c.boxMic ? Command::BoxMic : Command::Mic),
        command(Command::AudioTransferOff),
        // O node-CarPlay manda WifiConnect 1 s depois; quem chama cuida do atraso.
    };
}

std::optional<Header> parseHeader(const QByteArray &b)
{
    if (b.size() < HeaderSize || getU32(b, 0) != Magic) return std::nullopt;
    const uint32_t type = getU32(b, 8);
    if (getU32(b, 12) != ~type) return std::nullopt;
    return Header{getU32(b, 4), Type(type)};
}

std::optional<VideoFrame> parseVideo(const QByteArray &p)
{
    if (p.size() < 20) return std::nullopt;
    return VideoFrame{getU32(p, 0), getU32(p, 4), getU32(p, 8), p.mid(20)};
}

std::optional<AudioPacket> parseAudio(const QByteArray &p)
{
    if (p.size() < 12) return std::nullopt;
    AudioPacket a;
    a.decodeType = getU32(p, 0);
    a.volume = getF32(p, 4);
    a.audioType = getU32(p, 8);
    const int rest = int(p.size()) - 12;
    if (rest == 1) a.command = AudioCommand(int8_t(p.at(12)));
    else if (rest == 4) a.volumeDuration = getF32(p, 12);
    else a.pcm = p.mid(12);
    return a;
}

std::optional<PhoneType> parsePlugged(const QByteArray &p)
{
    if (p.size() < 4) return std::nullopt;
    return PhoneType(getU32(p, 0));
}

std::optional<uint32_t> parseU32(const QByteArray &p)
{
    if (p.size() < 4) return std::nullopt;
    return getU32(p, 0);
}

std::optional<AudioFormat> audioFormat(uint32_t decodeType)
{
    switch (decodeType) {
    case 1: case 2: return AudioFormat{44100, 2};
    case 3: return AudioFormat{8000, 1};
    case 4: return AudioFormat{48000, 2};
    case 5: return AudioFormat{16000, 1};
    case 6: return AudioFormat{24000, 1};
    case 7: return AudioFormat{16000, 2};
    }
    return std::nullopt;
}

QString phoneTypeName(PhoneType t)
{
    switch (t) {
    case PhoneType::CarPlay: return QStringLiteral("CarPlay");
    case PhoneType::AndroidAuto: return QStringLiteral("Android Auto");
    case PhoneType::HiCar: return QStringLiteral("HiCar");
    case PhoneType::AndroidMirror: return QStringLiteral("Espelhamento Android");
    case PhoneType::IPhoneMirror: return QStringLiteral("Espelhamento iPhone");
    }
    return QStringLiteral("Desconhecido");
}

} // namespace dongle
