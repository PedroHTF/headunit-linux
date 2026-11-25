#pragma once

#include "app/AppSettings.h"
#include "hal/PhoneHal.h"
#include "hal/RadioHal.h"
#include "hal/VehicleHal.h"
#include "media/MediaPlayerController.h"
#include "projection/ProjectionController.h"

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
    AppController(VehicleHal *vehicle, RadioHal *radio, MediaPlayerController *player,
                  AppSettings *settings, PhoneHal *phone = nullptr, QObject *parent = nullptr);

    /** "none", "media", "radio" ou "projection". */
    QString audioSource() const { return m_source; }
    bool videoAllowed() const;
    bool showReverseCamera() const;

    /** Mesmo efeito de um botão do volante: "next", "previous", "playpause"... */
    Q_INVOKABLE void pressKey(const QString &key);
    Q_INVOKABLE void onRadioStarted();
    void handleKey(VehicleHal::Key key);
    /** CarPlay/Android Auto: botões do volante vão para o celular quando ele toca. */
    void setProjection(ProjectionController *projection);

signals:
    void audioSourceChanged();
    void drivingStateChanged();

private:
    void setSource(const QString &s);
    void onVehicleChanged();
    void onPhoneChanged();

    VehicleHal *m_vehicle;
    RadioHal *m_radio;
    MediaPlayerController *m_player;
    AppSettings *m_settings;
    PhoneHal *m_phone;
    ProjectionController *m_projection = nullptr;
    // O que estava tocando antes da ligação, para retomar depois.
    bool m_inCall = false;
    bool m_resumeMedia = false;
    bool m_resumeRadio = false;
    QString m_source = QStringLiteral("none");
    bool m_wasPowered = true;
};
