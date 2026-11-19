#include "logo/LogoManager.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QtGlobal>

static const QStringList kImageExt = {"png", "jpg", "jpeg", "webp", "bmp"};
static const QStringList kVideoExt = {"mp4", "mkv", "webm", "mov", "avi"};

LogoManager::LogoManager(QString dataDir, QSettings *storage, QObject *parent)
    : QObject(parent), m_dataDir(std::move(dataDir)), m_default(QStringLiteral("HeadUnit"), QStringLiteral("HeadUnit"))
{
    m_storage = storage ? storage : &m_default;
    m_kind = m_storage->value(QStringLiteral("logo/kind"), m_kind).toString();
    m_ext = m_storage->value(QStringLiteral("logo/ext")).toString();
    m_text = m_storage->value(QStringLiteral("logo/text"), m_text).toString();
    m_seconds = m_storage->value(QStringLiteral("logo/seconds"), m_seconds).toInt();
    m_onIgnition = m_storage->value(QStringLiteral("logo/onIgnition"), m_onIgnition).toBool();
    if (m_kind != u"default" && !QFile::exists(storedPath()))
        m_kind = QStringLiteral("default");
}

QString LogoManager::storedPath() const { return QDir(m_dataDir).filePath(QStringLiteral("boot_logo.") + m_ext); }

QUrl LogoManager::fileUrl() const
{
    if (m_kind == u"default") return {};
    QUrl url = QUrl::fromLocalFile(storedPath());
    url.setQuery(QStringLiteral("v=%1").arg(m_version)); // força recarregar após trocar
    return url;
}

bool LogoManager::isVideoFile(const QString &path) { return kVideoExt.contains(QFileInfo(path).suffix().toLower()); }

QVariantList LogoManager::findCandidates(const QString &root) const
{
    QVariantList out;
    QDirIterator dirs(root, QDir::Dirs | QDir::NoDotAndDotDot);
    while (dirs.hasNext()) {
        const QString dir = dirs.next();
        if (QFileInfo(dir).fileName().compare(QStringLiteral("HeadUnit"), Qt::CaseInsensitive) != 0) continue;
        QDirIterator files(dir, QDir::Files);
        while (files.hasNext()) {
            const QFileInfo fi(files.next());
            const QString ext = fi.suffix().toLower();
            if (fi.completeBaseName().compare(QStringLiteral("logo"), Qt::CaseInsensitive) != 0) continue;
            if (!kImageExt.contains(ext) && !kVideoExt.contains(ext)) continue;
            out << QVariantMap{{"path", fi.absoluteFilePath()}, {"name", fi.fileName()}, {"isVideo", kVideoExt.contains(ext)}};
        }
    }
    return out;
}

bool LogoManager::importFile(const QString &path)
{
    const QFileInfo fi(path);
    const QString ext = fi.suffix().toLower();
    if (!fi.isFile() || (!kImageExt.contains(ext) && !kVideoExt.contains(ext))) return false;
    QDir().mkpath(m_dataDir);
    if (m_kind != u"default") QFile::remove(storedPath());
    m_ext = ext;
    QFile::remove(storedPath());
    if (!QFile::copy(path, storedPath())) return false;
    m_kind = kVideoExt.contains(ext) ? QStringLiteral("video") : QStringLiteral("image");
    ++m_version;
    save();
    return true;
}

void LogoManager::useDefault()
{
    if (m_kind != u"default") QFile::remove(storedPath());
    m_kind = QStringLiteral("default");
    ++m_version;
    save();
}

void LogoManager::setText(const QString &t)
{
    if (t == m_text) return;
    m_text = t.left(12);
    save();
}

void LogoManager::setSeconds(int s)
{
    s = qBound(1, s, 8);
    if (s == m_seconds) return;
    m_seconds = s;
    save();
}

void LogoManager::setOnIgnition(bool v)
{
    if (v == m_onIgnition) return;
    m_onIgnition = v;
    save();
}

void LogoManager::save()
{
    m_storage->setValue(QStringLiteral("logo/kind"), m_kind);
    m_storage->setValue(QStringLiteral("logo/ext"), m_ext);
    m_storage->setValue(QStringLiteral("logo/text"), m_text);
    m_storage->setValue(QStringLiteral("logo/seconds"), m_seconds);
    m_storage->setValue(QStringLiteral("logo/onIgnition"), m_onIgnition);
    emit changed();
}
