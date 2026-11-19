#include "app/AppSettings.h"

#include <QTime>

AppSettings::AppSettings(QSettings *storage, QObject *parent)
    : QObject(parent), m_default(QStringLiteral("HeadUnit"), QStringLiteral("HeadUnit"))
{
    m_storage = storage ? storage : &m_default;
    m_videoWhileDriving = m_storage->value(QStringLiteral("video/whileDriving"), false).toBool();
    m_themeMode = m_storage->value(QStringLiteral("theme/mode"), m_themeMode).toString();
    m_highContrast = m_storage->value(QStringLiteral("theme/highContrast"), false).toBool();

    m_clock.setInterval(60 * 1000);
    connect(&m_clock, &QTimer::timeout, this, [this] {
        if (m_themeMode == u"auto") emit changed();
    });
    m_clock.start();
}

void AppSettings::setVideoWhileDriving(bool v)
{
    if (v == m_videoWhileDriving) return;
    m_videoWhileDriving = v;
    m_storage->setValue(QStringLiteral("video/whileDriving"), v);
    emit changed();
}

void AppSettings::setThemeMode(const QString &mode)
{
    const QString m = (mode == u"light" || mode == u"dark") ? mode : QStringLiteral("auto");
    if (m == m_themeMode) return;
    m_themeMode = m;
    if (m_persist) m_storage->setValue(QStringLiteral("theme/mode"), m);
    emit changed();
}

void AppSettings::setHighContrast(bool v)
{
    if (v == m_highContrast) return;
    m_highContrast = v;
    if (m_persist) m_storage->setValue(QStringLiteral("theme/highContrast"), v);
    emit changed();
}

bool AppSettings::dark() const
{
    if (m_themeMode == u"dark") return true;
    if (m_themeMode == u"light") return false;
    return isNight(QTime::currentTime().hour());
}

void AppSettings::overrideTheme(const QString &mode, bool highContrast)
{
    m_persist = false;
    setThemeMode(mode);
    setHighContrast(highContrast);
}
