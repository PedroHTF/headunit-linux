#pragma once

#include "hal/RadioHal.h"
#include "hal/VehicleHal.h"
#include "media/MediaPlayerController.h"

#include <QObject>

/**
 * "Cérebro" do app: fonte de áudio ativa, botões do volante, ACC e regras de direção.
 * Não sabe se a HAL é simulada ou real.
 */
class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString audioSource READ audioSource NOTIFY audioSourceChanged)
    Q_PROPERTY(bool videoAllowed READ videoAllowed NOTIFY drivingStateChanged)
    Q_PROPERTY(bool showReverseCamera READ showReverseCamera NOTIFY drivingStateChanged)

public:
    AppController(VehicleHal *vehicle, RadioHal *radio, MediaPlayerController *player, QObject *parent = nullptr);

    /** "none", "media" ou "radio". */
    QString audioSource() const { return m_source; }
    bool videoAllowed() const;
    bool showReverseCamera() const;

    /** Mesmo efeito de um botão do volante: "next", "previous", "playpause"... */
    Q_INVOKABLE void pressKey(const QString &key);
    Q_INVOKABLE void onRadioStarted();
    void handleKey(VehicleHal::Key key);

signals:
    void audioSourceChanged();
    void drivingStateChanged();

private:
    void setSource(const QString &s);
    void onVehicleChanged();

    VehicleHal *m_vehicle;
    RadioHal *m_radio;
    MediaPlayerController *m_player;
    QString m_source = QStringLiteral("none");
    bool m_wasPowered = true;
};
