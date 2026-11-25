#pragma once

#include "projection/DongleTransport.h"

#include <QTimer>
#include <atomic>
#include <mutex>
#include <thread>

struct libusb_context;
struct libusb_device_handle;

/**
 * Dongle real via libusb (VID 0x1314, PID 0x1520/0x1521). Procura o dongle a cada
 * 2 s; ao achar, lê mensagens numa thread (cabeçalho de 16 bytes + conteúdo).
 *
 * Linux: precisa de permissão no USB (regra udev, ver docs/dongle.md).
 * Windows: precisa do driver WinUSB instalado para o dongle (Zadig).
 */
class UsbDongleTransport : public DongleTransport
{
    Q_OBJECT
public:
    explicit UsbDongleTransport(QObject *parent = nullptr);
    ~UsbDongleTransport() override;

    bool isSimulated() const override { return false; }
    void start() override;
    void stop() override;
    void write(const QByteArray &message) override;

private:
    void tryOpen();
    void readLoop();
    void closeDevice();
    bool readExact(unsigned char *buf, int len);

    libusb_context *m_ctx = nullptr;
    libusb_device_handle *m_dev = nullptr;
    unsigned char m_in = 0, m_out = 0;
    std::thread m_reader;
    std::atomic<bool> m_reading{false};
    std::mutex m_writeLock;
    QTimer m_poll;
};
