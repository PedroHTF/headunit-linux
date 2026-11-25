#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <cstdint>
#include <optional>

/**
 * Protocolo USB do dongle CarPlay/Android Auto (Carlinkit e compatíveis), conforme a
 * engenharia reversa do projeto aberto node-CarPlay (licença MIT).
 *
 * Cada mensagem: cabeçalho de 16 bytes (uint32 little-endian: magic 0x55AA55AA,
 * tamanho do conteúdo, tipo, ~tipo) seguido do conteúdo.
 */
namespace dongle {

constexpr uint32_t Magic = 0x55AA55AA;
constexpr int HeaderSize = 16;
constexpr uint16_t VendorId = 0x1314;
constexpr uint16_t ProductIds[] = {0x1520, 0x1521};
constexpr int HeartbeatMs = 2000;

enum class Type : uint32_t {
    Open = 0x01, Plugged = 0x02, Phase = 0x03, Unplugged = 0x04, Touch = 0x05,
    VideoData = 0x06, AudioData = 0x07, Command = 0x08, LogoType = 0x09,
    BluetoothAddress = 0x0A, BluetoothPIN = 0x0C, BluetoothDeviceName = 0x0D,
    WifiDeviceName = 0x0E, DisconnectPhone = 0x0F, BluetoothPairedList = 0x12,
    ManufacturerInfo = 0x14, CloseDongle = 0x15, MultiTouch = 0x17, HiCarLink = 0x18,
    BoxSettings = 0x19, MediaData = 0x2A, SendFile = 0x99, HeartBeat = 0xAA,
    SoftwareVersion = 0xCC,
};

/** Comandos (CommandMapping). Só os usados aqui. */
enum class Command : uint32_t {
    Mic = 7, BoxMic = 15, EnableNightMode = 16, DisableNightMode = 17,
    AudioTransferOn = 22, AudioTransferOff = 23, Wifi24g = 24, Wifi5g = 25,
    Left = 100, Right = 101, SelectDown = 104, SelectUp = 105, Back = 106,
    Home = 200, Play = 201, Pause = 202, PlayOrPause = 203, Next = 204, Prev = 205,
    AcceptPhone = 300, RejectPhone = 301, Siri = 5,
    RequestVideoFocus = 500, ReleaseVideoFocus = 501,
    WifiEnable = 1000, WifiConnect = 1002,
};

enum class TouchAction : uint32_t { Down = 14, Move = 15, Up = 16 };

/** Tipo de celular informado na mensagem Plugged. */
enum class PhoneType : uint32_t { AndroidMirror = 1, CarPlay = 3, IPhoneMirror = 4, AndroidAuto = 5, HiCar = 6 };

/** Comando de áudio (quando o AudioData traz 1 byte em vez de PCM). */
enum class AudioCommand : int8_t {
    OutputStart = 1, OutputStop = 2, InputConfig = 3, PhonecallStart = 4, PhonecallStop = 5,
    NaviStart = 6, NaviStop = 7, SiriStart = 8, SiriStop = 9, MediaStart = 10, MediaStop = 11,
    AlertStart = 12, AlertStop = 13,
};

struct Header
{
    uint32_t length = 0;
    Type type = Type::HeartBeat;
};

struct Config
{
    int width = 800;
    int height = 480;
    int fps = 30;
    int dpi = 160;
    bool nightMode = false;
    bool rightHandDrive = false;
    QString boxName = QStringLiteral("HeadUnit");
    bool wifi5g = true;
    bool boxMic = false; // false: microfone da central
};

struct VideoFrame
{
    uint32_t width = 0, height = 0, flags = 0;
    QByteArray h264;
};

struct AudioPacket
{
    uint32_t decodeType = 0;
    float volume = 0;
    uint32_t audioType = 0;
    std::optional<AudioCommand> command;
    std::optional<float> volumeDuration;
    QByteArray pcm; // S16LE
};

struct AudioFormat { int sampleRate; int channels; };

// ---- montagem (central → dongle)
QByteArray encode(Type type, const QByteArray &payload = {});
QByteArray open(const Config &c);
/** [x], [y] normalizados de 0 a 1. */
QByteArray touch(TouchAction action, double x, double y);
QByteArray command(Command c);
QByteArray heartbeat();
QByteArray sendFile(const QByteArray &path, const QByteArray &content);
QByteArray sendNumber(const QByteArray &path, uint32_t value);
QByteArray sendString(const QByteArray &path, const QByteArray &value);
QByteArray boxSettings(int width, int height, qint64 nowMs, int mediaDelayMs = 300);
/** Mensagens enviadas ao conectar, na ordem do node-CarPlay. */
QList<QByteArray> startupSequence(const Config &c, qint64 nowMs);

// ---- leitura (dongle → central)
std::optional<Header> parseHeader(const QByteArray &bytes);
std::optional<VideoFrame> parseVideo(const QByteArray &payload);
std::optional<AudioPacket> parseAudio(const QByteArray &payload);
std::optional<PhoneType> parsePlugged(const QByteArray &payload);
std::optional<uint32_t> parseU32(const QByteArray &payload);
std::optional<AudioFormat> audioFormat(uint32_t decodeType);

QString phoneTypeName(PhoneType t);

} // namespace dongle
