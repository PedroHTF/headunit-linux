#pragma once

#include "projection/DongleProtocol.h"

#include <QImage>
#include <QObject>

/** Meio de comunicação com o dongle: USB de verdade ou simulado. */
class DongleTransport : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    virtual bool isSimulated() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    /** Envia uma mensagem já montada (dongle::encode...). */
    virtual void write(const QByteArray &message) = 0;

signals:
    /** Dongle encontrado e aberto. */
    void opened();
    /** Dongle removido ou fechado. */
    void closed();
    void message(dongle::Type type, const QByteArray &payload);
    /** Só o simulado: quadro já pronto (sem H.264). */
    void frameReady(const QImage &frame);
    void error(const QString &text);
};
