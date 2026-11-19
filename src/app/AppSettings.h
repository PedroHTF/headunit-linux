#pragma once

#include <QObject>
#include <QSettings>
#include <QTimer>

/** Configurações do usuário, salvas entre execuções (registro no Windows, ~/.config no Linux). */
class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool videoWhileDriving READ videoWhileDriving WRITE setVideoWhileDriving NOTIFY changed)
    /** "auto", "light" ou "dark". */
    Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY changed)
    Q_PROPERTY(bool highContrast READ highContrast WRITE setHighContrast NOTIFY changed)
    /** Tema escuro em uso agora (resolve o "auto"). */
    Q_PROPERTY(bool dark READ dark NOTIFY changed)

public:
    /** @param storage permite usar um arquivo próprio nos testes. */
    explicit AppSettings(QSettings *storage = nullptr, QObject *parent = nullptr);

    bool videoWhileDriving() const { return m_videoWhileDriving; }
    void setVideoWhileDriving(bool v);

    QString themeMode() const { return m_themeMode; }
    void setThemeMode(const QString &mode);
    bool highContrast() const { return m_highContrast; }
    void setHighContrast(bool v);
    bool dark() const;

    /** Automático: escuro das 18h às 6h. Na central, o ideal é o sinal do farol (CAN). */
    static bool isNight(int hour) { return hour >= 18 || hour < 6; }

    /** Muda o tema só nesta execução, sem salvar (usado nas capturas de tela). */
    void overrideTheme(const QString &mode, bool highContrast);

signals:
    void changed();

private:
    QSettings *m_storage;
    QSettings m_default;
    bool m_videoWhileDriving = false;
    QString m_themeMode = QStringLiteral("auto");
    bool m_highContrast = false;
    bool m_persist = true;
    QTimer m_clock; // reavalia o "auto" a cada minuto
};
