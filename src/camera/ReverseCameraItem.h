#pragma once

#include "camera/CameraSettings.h"

#include <QImage>
#include <QPointer>
#include <QQuickPaintedItem>

/**
 * Imagem da câmera de ré + linhas-guia, para usar no QML (`ReverseCamera { ... }`).
 * Hoje a imagem é simulada; com câmera real, o mesmo mapa de correção é aplicado aos quadros.
 */
class ReverseCameraItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(CameraSettings *settings READ settings WRITE setSettings NOTIFY settingsChanged)
    Q_PROPERTY(double steeringDeg READ steeringDeg WRITE setSteeringDeg NOTIFY steeringDegChanged)

public:
    static constexpr int ImageWidth = 480;
    static constexpr int ImageHeight = 270;

    explicit ReverseCameraItem(QQuickItem *parent = nullptr);

    CameraSettings *settings() const { return m_settings; }
    void setSettings(CameraSettings *s);
    double steeringDeg() const { return m_steeringDeg; }
    void setSteeringDeg(double d);

    void paint(QPainter *painter) override;

    /** Gera a imagem (simulada) já com ou sem fisheye. Público para testes/capturas. */
    static QImage renderImage(const CameraSettings &s);

signals:
    void settingsChanged();
    void steeringDegChanged();

private:
    QPointer<CameraSettings> m_settings;
    double m_steeringDeg = 0;
    QImage m_image;
    bool m_imageDirty = true;
};
