#include "media/MediaLibrary.h"

#include "media/FolderTree.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSet>
#include <algorithm>

static const QSet<QString> kAudio = {"mp3", "flac", "ogg", "oga", "opus", "wav", "m4a", "aac", "wma"};
static const QSet<QString> kVideo = {"mp4", "mkv", "avi", "mov", "webm", "m4v", "wmv"};

MediaLibrary::MediaLibrary(QObject *parent) : QAbstractListModel(parent)
{
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(300);
    connect(&m_debounce, &QTimer::timeout, this, &MediaLibrary::rescan);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, &m_debounce, qOverload<>(&QTimer::start));
}

bool MediaLibrary::isAudioFile(const QString &suffix) { return kAudio.contains(suffix.toLower()); }
bool MediaLibrary::isVideoFile(const QString &suffix) { return kVideo.contains(suffix.toLower()); }

void MediaLibrary::setRootPath(const QString &path)
{
    if (path == m_root) return;
    m_root = path;
    emit rootPathChanged();
    rescan();
}

void MediaLibrary::setMounted(bool mounted)
{
    if (mounted == m_mounted) return;
    m_mounted = mounted;
    emit mountedChanged();
    rescan();
}

void MediaLibrary::setShowVideos(bool v)
{
    if (v == m_showVideos) return;
    m_showVideos = v;
    emit showVideosChanged();
    applyFilter();
}

void MediaLibrary::rescan()
{
    QVector<MediaItem> items;
    if (m_mounted && !m_root.isEmpty() && QDir(m_root).exists()) {
        QDirIterator it(m_root, QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QFileInfo fi(it.next());
            const QString suffix = fi.suffix();
            const bool video = isVideoFile(suffix);
            if (!video && !isAudioFile(suffix))
                continue;
            const QString folder = FolderTree::normalize(QDir(m_root).relativeFilePath(fi.absolutePath()));
            if (folder.section(QLatin1Char('/'), 0, 0).compare(systemFolder(), Qt::CaseInsensitive) == 0)
                continue; // HeadUnit/ (logo) não entra na biblioteca
            const QString parent = fi.dir().dirName();
            items.push_back({fi.completeBaseName(),
                             fi.dir() == QDir(m_root) ? QStringLiteral("USB") : parent,
                             QUrl::fromLocalFile(fi.absoluteFilePath()), video,
                             folder == u"." ? QString() : folder});
        }
    }
    std::sort(items.begin(), items.end(), [](const MediaItem &a, const MediaItem &b) {
        return a.title.compare(b.title, Qt::CaseInsensitive) < 0;
    });
    m_all = items;
    applyFilter();
    rewatch();
}

void MediaLibrary::applyFilter()
{
    beginResetModel();
    m_visible.clear();
    for (const MediaItem &m : std::as_const(m_all))
        if (m.isVideo == m_showVideos)
            m_visible.push_back(m);
    endResetModel();
    emit countChanged();
}

void MediaLibrary::rewatch()
{
    if (!m_watcher.directories().isEmpty())
        m_watcher.removePaths(m_watcher.directories());
    if (m_root.isEmpty() || !QDir(m_root).exists())
        return;
    QStringList dirs{m_root};
    QDirIterator it(m_root, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
        dirs << it.next();
    m_watcher.addPaths(dirs);
}

QVector<MediaItem> MediaLibrary::filesIn(const QString &path) const
{
    const QString p = FolderTree::normalize(path);
    QVector<MediaItem> out;
    for (const MediaItem &m : m_all)
        if (m.folder == p) out << m;
    return out;
}

QVariantList MediaLibrary::folderEntries(const QString &path) const
{
    QStringList folders;
    for (const MediaItem &m : m_all) folders << m.folder;
    QVariantList out;
    for (const QString &name : FolderTree::subfolders(folders, path)) {
        const QString full = FolderTree::join(path, name);
        out << QVariantMap{{"kind", "folder"}, {"name", name}, {"path", full},
                           {"count", FolderTree::countUnder(folders, full)}};
    }
    const QVector<MediaItem> files = filesIn(path);
    for (int i = 0; i < files.size(); ++i) {
        out << QVariantMap{{"kind", "file"}, {"title", files[i].title}, {"subtitle", files[i].subtitle},
                           {"isVideo", files[i].isVideo}, {"index", i}};
    }
    return out;
}

QString MediaLibrary::parentFolder(const QString &path) const { return FolderTree::parent(path); }
QString MediaLibrary::joinFolder(const QString &path, const QString &child) const { return FolderTree::join(path, child); }

int MediaLibrary::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_visible.size());
}

QVariant MediaLibrary::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_visible.size())
        return {};
    const MediaItem &m = m_visible.at(index.row());
    switch (role) {
    case TitleRole: return m.title;
    case SubtitleRole: return m.subtitle;
    case UrlRole: return m.url;
    case IsVideoRole: return m.isVideo;
    }
    return {};
}

QHash<int, QByteArray> MediaLibrary::roleNames() const
{
    return {{TitleRole, "title"}, {SubtitleRole, "subtitle"}, {UrlRole, "url"}, {IsVideoRole, "isVideo"}};
}
