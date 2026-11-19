#pragma once

#include <QObject>
#include <QSettings>
#include <QUrl>
#include <QVariantList>

/**
 * Logo da inicialização: o padrão (desenhado em QML) ou uma imagem/vídeo do usuário.
 * O arquivo escolhido é copiado para os dados do app, então funciona sem o pendrive.
 * Convenção no pendrive: pasta `HeadUnit/` com `logo.png|jpg|webp|mp4`.
 */
class LogoManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString kind READ kind NOTIFY changed)           // "default", "image", "video"
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY changed)
    Q_PROPERTY(int version READ version NOTIFY changed)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY changed)
    Q_PROPERTY(int seconds READ seconds WRITE setSeconds NOTIFY changed)
    Q_PROPERTY(bool onIgnition READ onIgnition WRITE setOnIgnition NOTIFY changed)

public:
    LogoManager(QString dataDir, QSettings *storage = nullptr, QObject *parent = nullptr);

    QString kind() const { return m_kind; }
    QUrl fileUrl() const;
    int version() const { return m_version; }
    QString text() const { return m_text; }
    void setText(const QString &t);
    int seconds() const { return m_seconds; }
    void setSeconds(int s);
    bool onIgnition() const { return m_onIgnition; }
    void setOnIgnition(bool v);

    /** Procura HeadUnit/logo.* na pasta [root] (o pendrive). Lista de {path, name, isVideo}. */
    Q_INVOKABLE QVariantList findCandidates(const QString &root) const;
    Q_INVOKABLE bool importFile(const QString &path);
    /** Para o seletor de arquivos do QML (URL file:///...). */
    Q_INVOKABLE bool importUrl(const QUrl &url) { return importFile(url.toLocalFile()); }
    Q_INVOKABLE void useDefault();

    static bool isVideoFile(const QString &path);

signals:
    void changed();

private:
    QString storedPath() const;
    void save();

    QString m_dataDir;
    QSettings *m_storage;
    QSettings m_default;
    QString m_kind = QStringLiteral("default");
    QString m_ext;
    QString m_text = QStringLiteral("HR-V");
    int m_seconds = 3;
    bool m_onIgnition = true;
    int m_version = 0;
};
