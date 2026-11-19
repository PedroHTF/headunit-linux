#pragma once

#include "camera/CameraModel.h"
#include "camera/Guidelines.h"
#include "camera/SyntheticScene.h"

#include <QObject>
#include <QSettings>
#include <QVariantMap>

/**
 * Configuração da câmera de ré (carro, posição da câmera, lente, linhas), salva entre
 * execuções. No QML: `cameraSettings.values["cam.height"]` e `cameraSettings.set(...)`.
 */
class CameraSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap values READ values NOTIFY changed)

public:
    explicit CameraSettings(QSettings *storage = nullptr, QObject *parent = nullptr);

    QVariantMap values() const { return m_values; }

    Q_INVOKABLE void set(const QString &key, const QVariant &value);
    /** Liga/desliga um valor numa lista (marcas de distância). */
    Q_INVOKABLE void toggleInList(const QString &key, double value);
    Q_INVOKABLE void reset();

    static QVariantMap defaults();

    cam::CameraPose pose() const;
    cam::Lens lens() const;
    cam::Lens correctedLens() const;
    bool removeFisheye() const;
    cam::VehicleDimensions car() const;
    cam::GuideOptions guides() const;
    cam::Scene scene() const;

signals:
    void changed();
    /** Só o que muda a imagem (não as linhas). */
    void imageChanged();

private:
    double d(const char *key) const { return m_values.value(QLatin1String(key)).toDouble(); }
    bool b(const char *key) const { return m_values.value(QLatin1String(key)).toBool(); }

    QSettings *m_storage;
    QSettings m_default;
    QVariantMap m_values;
};
