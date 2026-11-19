#pragma once

#include <QObject>
#include <QSettings>

/** Configurações do usuário, salvas entre execuções (registro no Windows, ~/.config no Linux). */
class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool videoWhileDriving READ videoWhileDriving WRITE setVideoWhileDriving NOTIFY changed)

public:
    /** @param storage permite usar um arquivo próprio nos testes. */
    explicit AppSettings(QSettings *storage = nullptr, QObject *parent = nullptr);

    bool videoWhileDriving() const { return m_videoWhileDriving; }
    void setVideoWhileDriving(bool v);

signals:
    void changed();

private:
    QSettings *m_storage;
    QSettings m_default;
    bool m_videoWhileDriving = false;
};
