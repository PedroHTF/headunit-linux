#include "media/MediaPlayerController.h"

#include <QtGlobal>

MediaPlayerController::MediaPlayerController(MediaLibrary *library, QObject *parent)
    : QObject(parent), m_library(library)
{
    m_player.setAudioOutput(&m_audio);
    m_audio.setVolume(0.6);

    connect(&m_player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayerController::playingChanged);
    connect(&m_player, &QMediaPlayer::positionChanged, this, &MediaPlayerController::positionChanged);
    connect(&m_player, &QMediaPlayer::durationChanged, this, &MediaPlayerController::durationChanged);
    connect(&m_audio, &QAudioOutput::volumeChanged, this, &MediaPlayerController::volumeChanged);
    connect(&m_audio, &QAudioOutput::mutedChanged, this, &MediaPlayerController::volumeChanged);
    connect(&m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus s) {
        if (s == QMediaPlayer::EndOfMedia && hasNext())
            next();
    });
    connect(&m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &msg) {
        m_error = msg;
        emit errorChanged();
    });
}

void MediaPlayerController::playFromLibrary(int index)
{
    setPlaylist(m_library->visibleItems(), index);
}

void MediaPlayerController::setPlaylist(const QVector<MediaItem> &items, int index)
{
    m_playlist = items;
    if (index < 0 || index >= m_playlist.size()) {
        m_index = -1;
        m_player.stop();
        emit currentChanged();
        return;
    }
    load(index);
    emit playbackStarted();
    m_player.play();
}

void MediaPlayerController::load(int index)
{
    m_index = index;
    m_error.clear();
    emit errorChanged();
    m_player.setSource(m_playlist.at(index).url);
    emit currentChanged();
}

void MediaPlayerController::togglePlayPause()
{
    if (!hasMedia()) return;
    if (isPlaying()) {
        m_player.pause();
    } else {
        emit playbackStarted();
        m_player.play();
    }
}

void MediaPlayerController::pause() { m_player.pause(); }

void MediaPlayerController::next()
{
    if (!hasNext()) return;
    const bool wasPlaying = isPlaying() || m_player.mediaStatus() == QMediaPlayer::EndOfMedia;
    load(m_index + 1);
    if (wasPlaying) m_player.play();
}

void MediaPlayerController::previous()
{
    if (!hasMedia()) return;
    // Como num rádio de carro: volta ao início se já tocou mais de 3 s.
    if (m_player.position() > 3000 || m_index == 0) {
        m_player.setPosition(0);
        return;
    }
    const bool wasPlaying = isPlaying();
    load(m_index - 1);
    if (wasPlaying) m_player.play();
}

void MediaPlayerController::seek(qint64 ms) { m_player.setPosition(ms); }

void MediaPlayerController::setVideoOutput(QObject *output) { m_player.setVideoOutput(output); }

QString MediaPlayerController::title() const { return hasMedia() ? m_playlist.at(m_index).title : QString(); }
QString MediaPlayerController::subtitle() const { return hasMedia() ? m_playlist.at(m_index).subtitle : QString(); }
bool MediaPlayerController::isVideo() const { return hasMedia() && m_playlist.at(m_index).isVideo; }
bool MediaPlayerController::isPlaying() const { return m_player.playbackState() == QMediaPlayer::PlayingState; }

void MediaPlayerController::setVolume(double v) { m_audio.setVolume(qBound(0.0, v, 1.0)); }
void MediaPlayerController::setMuted(bool m) { m_audio.setMuted(m); }
