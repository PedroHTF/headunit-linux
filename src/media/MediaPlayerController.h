#pragma once

#include "media/MediaLibrary.h"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QObject>

/** Player de música e vídeo (QtMultimedia; FFmpeg/GStreamer por baixo). */
class MediaPlayerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY currentChanged)
    Q_PROPERTY(QString subtitle READ subtitle NOTIFY currentChanged)
    Q_PROPERTY(bool isVideo READ isVideo NOTIFY currentChanged)
    Q_PROPERTY(bool hasMedia READ hasMedia NOTIFY currentChanged)
    Q_PROPERTY(bool hasNext READ hasNext NOTIFY currentChanged)
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY volumeChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    explicit MediaPlayerController(MediaLibrary *library, QObject *parent = nullptr);

    /** Toca a lista visível da biblioteca a partir de [index]. */
    Q_INVOKABLE void playFromLibrary(int index);
    /** Toca a pasta inteira (músicas e vídeos) a partir de [index]. */
    Q_INVOKABLE void playFolder(const QString &path, int index);
    Q_INVOKABLE void togglePlayPause();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void seek(qint64 ms);
    /** Recebe o VideoOutput do QML. */
    Q_INVOKABLE void setVideoOutput(QObject *output);

    void setPlaylist(const QVector<MediaItem> &items, int index);

    QString title() const;
    QString subtitle() const;
    bool isVideo() const;
    bool hasMedia() const { return m_index >= 0; }
    bool hasNext() const { return m_index >= 0 && m_index + 1 < m_playlist.size(); }
    bool isPlaying() const;
    qint64 position() const { return m_player.position(); }
    qint64 duration() const { return m_player.duration(); }
    double volume() const { return m_audio.volume(); }
    void setVolume(double v);
    bool isMuted() const { return m_audio.isMuted(); }
    void setMuted(bool m);
    QString error() const { return m_error; }

signals:
    void currentChanged();
    void playingChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void errorChanged();
    void playbackStarted();

private:
    void load(int index);

    MediaLibrary *m_library;
    QMediaPlayer m_player;
    QAudioOutput m_audio;
    QVector<MediaItem> m_playlist;
    int m_index = -1;
    QString m_error;
};
