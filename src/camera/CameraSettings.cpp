#include "camera/CameraSettings.h"

#include <algorithm>

CameraSettings::CameraSettings(QSettings *storage, QObject *parent)
    : QObject(parent), m_default(QStringLiteral("HeadUnit"), QStringLiteral("HeadUnit"))
{
    m_storage = storage ? storage : &m_default;
    m_values = defaults();
    m_storage->beginGroup(QStringLiteral("camera"));
    for (const QString &key : m_storage->allKeys()) {
        if (!m_values.contains(key)) continue;
        QVariant v = m_storage->value(key);
        const QVariant def = m_values.value(key);
        if (def.typeId() == QMetaType::Bool) v = v.toBool();
        else if (def.typeId() == QMetaType::Double) v = v.toDouble();
        else if (def.typeId() == QMetaType::QVariantList) {
            QVariantList list;
            for (const QVariant &x : v.toList()) list << x.toDouble();
            v = list;
        }
        m_values.insert(key, v);
    }
    m_storage->endGroup();
}

QVariantMap CameraSettings::defaults()
{
    return {
        {"lens.fov", 150.0}, {"lens.fisheye", 0.8},
        {"img.removeFisheye", false}, {"img.correctedFov", 110.0}, {"img.scene", QStringLiteral("parking")},
        {"cam.height", 0.9}, {"cam.pitch", 30.0}, {"cam.yaw", 0.0}, {"cam.roll", 0.0}, {"cam.offsetX", 0.0},
        {"car.width", 1.75}, {"car.length", 4.30}, {"car.wheelbase", 2.60}, {"car.overhang", 0.85},
        {"car.ratio", 15.0}, {"car.maxWheel", 35.0},
        {"lines.static", true}, {"lines.dynamic", true}, {"lines.invert", false},
        {"lines.dynLength", 3.0}, {"lines.offsetX", 0.0}, {"lines.offsetY", 0.0},
        {"lines.markers", QVariantList{0.5, 1.0, 2.0}}, {"lines.carMarkers", QVariantList{1.0}},
    };
}

void CameraSettings::set(const QString &key, const QVariant &value)
{
    if (!m_values.contains(key) || m_values.value(key) == value) return;
    m_values.insert(key, value);
    m_storage->setValue(QStringLiteral("camera/") + key, value);
    emit changed();
    if (key.startsWith(u"lens.") || key.startsWith(u"img.") || key.startsWith(u"cam."))
        emit imageChanged();
}

void CameraSettings::toggleInList(const QString &key, double value)
{
    QList<double> list;
    for (const QVariant &v : m_values.value(key).toList()) list << v.toDouble();
    if (list.contains(value)) list.removeAll(value);
    else list << value;
    std::sort(list.begin(), list.end());
    QVariantList out;
    for (double x : list) out << x;
    set(key, out);
}

void CameraSettings::reset()
{
    m_storage->remove(QStringLiteral("camera"));
    m_values = defaults();
    emit changed();
    emit imageChanged();
}

cam::CameraPose CameraSettings::pose() const
{
    return {d("cam.height"), d("cam.pitch"), d("cam.yaw"), d("cam.roll"), d("cam.offsetX")};
}

cam::Lens CameraSettings::lens() const { return {d("lens.fov"), d("lens.fisheye")}; }
cam::Lens CameraSettings::correctedLens() const { return {d("img.correctedFov"), 0.0}; }
bool CameraSettings::removeFisheye() const { return b("img.removeFisheye"); }

cam::VehicleDimensions CameraSettings::car() const
{
    return {d("car.width"), d("car.length"), d("car.wheelbase"), d("car.overhang"), d("car.ratio"), d("car.maxWheel")};
}

cam::GuideOptions CameraSettings::guides() const
{
    cam::GuideOptions o;
    o.showStatic = b("lines.static");
    o.showDynamic = b("lines.dynamic");
    o.invertSteering = b("lines.invert");
    o.dynamicLengthM = d("lines.dynLength");
    o.lineOffsetXM = d("lines.offsetX");
    o.lineOffsetYM = d("lines.offsetY");
    o.distanceMarkersM.clear();
    for (const QVariant &v : m_values.value(QStringLiteral("lines.markers")).toList()) o.distanceMarkersM.push_back(v.toDouble());
    o.carLengthMarkers.clear();
    for (const QVariant &v : m_values.value(QStringLiteral("lines.carMarkers")).toList()) o.carLengthMarkers.push_back(int(v.toDouble()));
    return o;
}

cam::Scene CameraSettings::scene() const
{
    return m_values.value(QStringLiteral("img.scene")).toString() == u"grid" ? cam::Scene::Grid : cam::Scene::Parking;
}
