#pragma once

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QList>
#include <QVariantMap>

/** Item das listas do oFono (GetModems, GetCalls): assinatura D-Bus (oa{sv}). */
struct OfonoObject
{
    QDBusObjectPath path;
    QVariantMap properties;
};
using OfonoObjectList = QList<OfonoObject>;

inline QDBusArgument &operator<<(QDBusArgument &arg, const OfonoObject &o)
{
    arg.beginStructure();
    arg << o.path << o.properties;
    arg.endStructure();
    return arg;
}

inline const QDBusArgument &operator>>(const QDBusArgument &arg, OfonoObject &o)
{
    arg.beginStructure();
    arg >> o.path >> o.properties;
    arg.endStructure();
    return arg;
}

Q_DECLARE_METATYPE(OfonoObject)
Q_DECLARE_METATYPE(OfonoObjectList)

inline void registerOfonoTypes()
{
    qDBusRegisterMetaType<OfonoObject>();
    qDBusRegisterMetaType<OfonoObjectList>();
}
