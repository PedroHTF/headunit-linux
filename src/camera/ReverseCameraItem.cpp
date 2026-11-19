#include "camera/ReverseCameraItem.h"

#include "camera/Undistort.h"

#include <QPainter>
#include <QPainterPath>

ReverseCameraItem::ReverseCameraItem(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
}

void ReverseCameraItem::setSettings(CameraSettings *s)
{
    if (s == m_settings) return;
    if (m_settings) disconnect(m_settings, nullptr, this, nullptr);
    m_settings = s;
    if (s) {
        connect(s, &CameraSettings::changed, this, [this] { update(); });
        connect(s, &CameraSettings::imageChanged, this, [this] { m_imageDirty = true; update(); });
    }
    m_imageDirty = true;
    emit settingsChanged();
    update();
}

void ReverseCameraItem::setSteeringDeg(double d)
{
    if (qFuzzyCompare(d + 1000, m_steeringDeg + 1000)) return;
    m_steeringDeg = d;
    emit steeringDegChanged();
    update();
}

QImage ReverseCameraItem::renderImage(const CameraSettings &s)
{
    const cam::CameraProjector raw(s.pose(), s.lens(), ImageWidth, ImageHeight);
    std::vector<uint32_t> px = cam::renderScene(raw, s.scene());
    if (s.removeFisheye()) {
        const cam::CameraProjector fixed(s.pose(), s.correctedLens(), ImageWidth, ImageHeight);
        px = cam::applyMap(cam::buildUndistortMap(raw, fixed), px);
    }
    QImage img(ImageWidth, ImageHeight, QImage::Format_ARGB32);
    for (int y = 0; y < ImageHeight; ++y)
        std::copy_n(px.data() + size_t(y) * ImageWidth, ImageWidth, reinterpret_cast<uint32_t *>(img.scanLine(y)));
    return img;
}

static QColor colorOf(cam::GuideColor c)
{
    switch (c) {
    case cam::GuideColor::Red: return QColor(0xF4, 0x43, 0x36);
    case cam::GuideColor::Yellow: return QColor(0xFF, 0xC1, 0x07);
    case cam::GuideColor::Green: return QColor(0x4C, 0xAF, 0x50);
    case cam::GuideColor::Blue: return QColor(0x29, 0xB6, 0xF6);
    case cam::GuideColor::White: break;
    }
    return Qt::white;
}

void ReverseCameraItem::paint(QPainter *p)
{
    p->fillRect(boundingRect(), Qt::black);
    if (!m_settings) return;
    if (m_imageDirty) {
        m_image = renderImage(*m_settings);
        m_imageDirty = false;
    }
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->drawImage(boundingRect(), m_image);

    const double sx = width() / ImageWidth, sy = height() / ImageHeight;
    const cam::CameraProjector display(m_settings->pose(),
                                       m_settings->removeFisheye() ? m_settings->correctedLens() : m_settings->lens(),
                                       ImageWidth, ImageHeight);
    QFont font = p->font();
    font.setBold(true);
    font.setPixelSize(std::max(10, int(height() / 28)));
    p->setFont(font);
    p->setRenderHint(QPainter::Antialiasing);

    for (const cam::GuideLine &line : cam::Guidelines::build(m_settings->car(), m_settings->guides(), m_steeringDeg)) {
        // Subdivide para acompanhar a curvatura do fisheye.
        QPainterPath path;
        bool started = false;
        QPointF first;
        for (size_t i = 0; i + 1 < line.points.size(); ++i) {
            const cam::Ground a = line.points[i], b = line.points[i + 1];
            for (int k = (i == 0 ? 0 : 1); k <= 12; ++k) {
                const auto px = display.project({a.x + (b.x - a.x) * k / 12, a.y + (b.y - a.y) * k / 12});
                if (!px) continue;
                const QPointF pt(px->u * sx, px->v * sy);
                if (!started) { path.moveTo(pt); first = pt; started = true; }
                else path.lineTo(pt);
            }
        }
        if (!started) continue;
        p->setPen(QPen(colorOf(line.color), std::max(2.0, height() / 110), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p->drawPath(path);
        if (!line.label.empty()) {
            const QString label = QString::fromStdString(line.label);
            const QPointF at(first.x() - p->fontMetrics().horizontalAdvance(label) - 8, first.y() + font.pixelSize() / 3.0);
            p->setPen(Qt::black);
            p->drawText(at + QPointF(1, 1), label);
            p->setPen(Qt::white);
            p->drawText(at, label);
        }
    }
}
