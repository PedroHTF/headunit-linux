#include "app/AppController.h"

#include "hal/DrivingRules.h"

AppController::AppController(VehicleHal *vehicle, RadioHal *radio, MediaPlayerController *player,
                             AppSettings *settings, QObject *parent)
    : QObject(parent), m_vehicle(vehicle), m_radio(radio), m_player(player), m_settings(settings)
{
    connect(m_settings, &AppSettings::changed, this, &AppController::drivingStateChanged);
    connect(m_vehicle, &VehicleHal::steeringKey, this, &AppController::handleKey);
    connect(m_vehicle, &VehicleHal::stateChanged, this, &AppController::onVehicleChanged);
    connect(m_player, &MediaPlayerController::playbackStarted, this, [this] {
        m_radio->powerOff();
        setSource(QStringLiteral("media"));
    });
    m_wasPowered = m_vehicle->powered();
}

bool AppController::videoAllowed() const { return DrivingRules::isVideoAllowed(m_vehicle->speedKmh(), m_settings->videoWhileDriving()); }

bool AppController::showReverseCamera() const
{
    return DrivingRules::shouldShowReverseCamera(m_vehicle->ignition(), m_vehicle->gear());
}

void AppController::onVehicleChanged()
{
    const bool powered = m_vehicle->powered();
    if (m_wasPowered && !powered) { // ACC desligado: para tudo
        m_player->pause();
        m_radio->powerOff();
    }
    m_wasPowered = powered;
    emit drivingStateChanged();
}

void AppController::onRadioStarted()
{
    m_player->pause();
    setSource(QStringLiteral("radio"));
}

void AppController::pressKey(const QString &key)
{
    VehicleHal::Key k;
    if (VehicleHal::keyFromString(key, &k))
        handleKey(k);
}

void AppController::handleKey(VehicleHal::Key key)
{
    const bool radio = m_source == u"radio";
    switch (key) {
    case VehicleHal::Key::Next:
        radio ? m_radio->seekUp() : m_player->next();
        break;
    case VehicleHal::Key::Previous:
        radio ? m_radio->seekDown() : m_player->previous();
        break;
    case VehicleHal::Key::PlayPause:
        if (radio)
            m_radio->isOn() ? m_radio->powerOff() : m_radio->powerOn();
        else
            m_player->togglePlayPause();
        break;
    case VehicleHal::Key::VolumeUp:
        m_player->setVolume(m_player->volume() + 0.05);
        break;
    case VehicleHal::Key::VolumeDown:
        m_player->setVolume(m_player->volume() - 0.05);
        break;
    case VehicleHal::Key::Mute:
        m_player->setMuted(!m_player->isMuted());
        break;
    case VehicleHal::Key::Mode:
        if (radio) {
            m_radio->powerOff();
            setSource(QStringLiteral("media"));
            m_player->togglePlayPause();
        } else {
            onRadioStarted();
            m_radio->powerOn();
        }
        break;
    }
}

void AppController::setSource(const QString &s)
{
    if (s == m_source) return;
    m_source = s;
    emit audioSourceChanged();
}
