#include "projection/UsbDongleTransport.h"

#include <QMetaObject>
#include <libusb.h>

using namespace dongle;

UsbDongleTransport::UsbDongleTransport(QObject *parent) : DongleTransport(parent)
{
    m_poll.setInterval(2000);
    connect(&m_poll, &QTimer::timeout, this, &UsbDongleTransport::tryOpen);
}

UsbDongleTransport::~UsbDongleTransport()
{
    stop();
    if (m_ctx) libusb_exit(m_ctx);
}

void UsbDongleTransport::start()
{
    if (!m_ctx && libusb_init(&m_ctx) != 0) {
        emit error(QStringLiteral("libusb indisponível"));
        return;
    }
    m_poll.start();
    tryOpen();
}

void UsbDongleTransport::stop()
{
    m_poll.stop();
    closeDevice();
}

void UsbDongleTransport::tryOpen()
{
    if (m_dev) return;
    for (uint16_t pid : ProductIds) {
        m_dev = libusb_open_device_with_vid_pid(m_ctx, VendorId, pid);
        if (m_dev) break;
    }
    if (!m_dev) return;

    libusb_set_auto_detach_kernel_driver(m_dev, 1);
    libusb_set_configuration(m_dev, 1);
    if (libusb_claim_interface(m_dev, 0) != 0) {
        emit error(QStringLiteral("Não foi possível usar o dongle (permissão USB?)"));
        libusb_close(m_dev);
        m_dev = nullptr;
        return;
    }
    // Primeiro endpoint de entrada e de saída da interface 0.
    libusb_config_descriptor *cfg = nullptr;
    m_in = m_out = 0;
    if (libusb_get_active_config_descriptor(libusb_get_device(m_dev), &cfg) == 0) {
        const libusb_interface_descriptor &alt = cfg->interface[0].altsetting[0];
        for (int i = 0; i < alt.bNumEndpoints; ++i) {
            const uint8_t addr = alt.endpoint[i].bEndpointAddress;
            if ((addr & LIBUSB_ENDPOINT_IN) && !m_in) m_in = addr;
            if (!(addr & LIBUSB_ENDPOINT_IN) && !m_out) m_out = addr;
        }
        libusb_free_config_descriptor(cfg);
    }
    if (!m_in || !m_out) {
        emit error(QStringLiteral("Endpoints do dongle não encontrados"));
        closeDevice();
        return;
    }
    m_reading = true;
    m_reader = std::thread(&UsbDongleTransport::readLoop, this);
    emit opened();
}

bool UsbDongleTransport::readExact(unsigned char *buf, int len)
{
    int got = 0;
    while (got < len && m_reading) {
        int n = 0;
        const int r = libusb_bulk_transfer(m_dev, m_in, buf + got, len - got, &n, 1000);
        if (r == LIBUSB_ERROR_TIMEOUT) continue;
        if (r != 0) return false;
        got += n;
    }
    return got == len;
}

void UsbDongleTransport::readLoop()
{
    int errors = 0;
    while (m_reading) {
        QByteArray head(HeaderSize, 0);
        if (!readExact(reinterpret_cast<unsigned char *>(head.data()), HeaderSize)) break;
        const auto h = parseHeader(head);
        if (!h) {
            if (++errors >= 5) break; // mesmo critério do node-CarPlay
            continue;
        }
        QByteArray body(int(h->length), 0);
        if (h->length && !readExact(reinterpret_cast<unsigned char *>(body.data()), int(h->length))) break;
        const Type type = h->type;
        QMetaObject::invokeMethod(this, [this, type, body] { emit message(type, body); }, Qt::QueuedConnection);
    }
    if (m_reading) // saiu por erro/desconexão
        QMetaObject::invokeMethod(this, [this] { closeDevice(); }, Qt::QueuedConnection);
}

void UsbDongleTransport::write(const QByteArray &message)
{
    std::lock_guard<std::mutex> lock(m_writeLock);
    if (!m_dev) return;
    int n = 0;
    libusb_bulk_transfer(m_dev, m_out, reinterpret_cast<unsigned char *>(const_cast<char *>(message.constData())),
                         int(message.size()), &n, 1000);
}

void UsbDongleTransport::closeDevice()
{
    const bool wasOpen = m_dev != nullptr;
    m_reading = false;
    if (m_reader.joinable()) m_reader.join();
    std::lock_guard<std::mutex> lock(m_writeLock);
    if (m_dev) {
        libusb_release_interface(m_dev, 0);
        libusb_close(m_dev);
        m_dev = nullptr;
    }
    if (wasOpen) emit closed();
}
