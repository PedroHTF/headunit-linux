#include "app/AppSettings.h"

AppSettings::AppSettings(QSettings *storage, QObject *parent)
    : QObject(parent), m_default(QStringLiteral("HeadUnit"), QStringLiteral("HeadUnit"))
{
    m_storage = storage ? storage : &m_default;
    m_videoWhileDriving = m_storage->value(QStringLiteral("video/whileDriving"), false).toBool();
}

void AppSettings::setVideoWhileDriving(bool v)
{
    if (v == m_videoWhileDriving) return;
    m_videoWhileDriving = v;
    m_storage->setValue(QStringLiteral("video/whileDriving"), v);
    emit changed();
}
