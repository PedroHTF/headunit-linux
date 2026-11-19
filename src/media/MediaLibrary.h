#pragma once

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QUrl>
#include <QVector>

struct MediaItem
{
    QString title;
    QString subtitle; // pasta (normalmente artista/álbum)
    QUrl url;
    bool isVideo = false;
};

/**
 * Lista músicas e vídeos de uma pasta: no PC ela faz o papel do pendrive USB.
 * Atualiza sozinha quando arquivos mudam. "mounted" simula plugar/tirar o pendrive.
 *
 * Na central real: a raiz passa a ser o ponto de montagem do pendrive (/media/usb...),
 * detectado via udev/udisks.
 */
class MediaLibrary : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString rootPath READ rootPath WRITE setRootPath NOTIFY rootPathChanged)
    Q_PROPERTY(bool mounted READ isMounted WRITE setMounted NOTIFY mountedChanged)
    Q_PROPERTY(bool showVideos READ showVideos WRITE setShowVideos NOTIFY showVideosChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role { TitleRole = Qt::UserRole + 1, SubtitleRole, UrlRole, IsVideoRole };

    explicit MediaLibrary(QObject *parent = nullptr);

    static bool isAudioFile(const QString &suffix);
    static bool isVideoFile(const QString &suffix);

    QString rootPath() const { return m_root; }
    void setRootPath(const QString &path);
    bool isMounted() const { return m_mounted; }
    void setMounted(bool mounted);
    bool showVideos() const { return m_showVideos; }
    void setShowVideos(bool v);
    int count() const { return int(m_visible.size()); }

    /** Itens visíveis (aba atual), na ordem da lista. */
    QVector<MediaItem> visibleItems() const { return m_visible; }
    /** Todos os itens (músicas + vídeos). */
    QVector<MediaItem> allItems() const { return m_all; }

    Q_INVOKABLE void rescan();

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void rootPathChanged();
    void mountedChanged();
    void showVideosChanged();
    void countChanged();

private:
    void applyFilter();
    void rewatch();

    QString m_root;
    bool m_mounted = true;
    bool m_showVideos = false;
    QVector<MediaItem> m_all;
    QVector<MediaItem> m_visible;
    QFileSystemWatcher m_watcher;
    QTimer m_debounce;
};
