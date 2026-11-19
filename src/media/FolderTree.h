#pragma once

#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <algorithm>

/**
 * Navegação por pastas a partir da lista de arquivos. Caminhos relativos à raiz do
 * pendrive, sem barras nas pontas; "" é a raiz.
 */
namespace FolderTree {

inline QString normalize(QString path)
{
    path.replace(QLatin1Char('\\'), QLatin1Char('/'));
    path.replace(QRegularExpression(QStringLiteral("/+")), QStringLiteral("/"));
    while (path.startsWith(QLatin1Char('/'))) path.remove(0, 1);
    while (path.endsWith(QLatin1Char('/'))) path.chop(1);
    return path;
}

/** Subpastas diretas de [path] (só as que têm mídia em algum nível abaixo). */
inline QStringList subfolders(const QStringList &folders, const QString &path)
{
    const QString base = normalize(path);
    const QString prefix = base.isEmpty() ? QString() : base + QLatin1Char('/');
    QSet<QString> seen;
    QStringList out;
    for (const QString &raw : folders) {
        const QString f = normalize(raw);
        if (!f.startsWith(prefix) || f.size() <= prefix.size()) continue;
        const QString child = f.mid(prefix.size()).section(QLatin1Char('/'), 0, 0);
        if (!seen.contains(child)) { seen.insert(child); out << child; }
    }
    std::sort(out.begin(), out.end(), [](const QString &a, const QString &b) { return a.compare(b, Qt::CaseInsensitive) < 0; });
    return out;
}

/** Pasta de cima; vazio na raiz (use [hasParent] para saber se existe). */
inline QString parent(const QString &path)
{
    const QString p = normalize(path);
    const int i = p.lastIndexOf(QLatin1Char('/'));
    return i < 0 ? QString() : p.left(i);
}

inline bool hasParent(const QString &path) { return !normalize(path).isEmpty(); }

inline QString join(const QString &path, const QString &child)
{
    return normalize(path.isEmpty() ? child : path + QLatin1Char('/') + child);
}

inline int countUnder(const QStringList &folders, const QString &path)
{
    const QString p = normalize(path);
    return int(std::count_if(folders.begin(), folders.end(), [&](const QString &raw) {
        const QString f = normalize(raw);
        return p.isEmpty() || f == p || f.startsWith(p + QLatin1Char('/'));
    }));
}

} // namespace FolderTree
