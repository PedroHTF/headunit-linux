#include "projection/ProjectionView.h"

#include <QPainter>

void ProjectionView::paint(QPainter *p)
{
    p->fillRect(boundingRect(), Qt::black);
    if (!m_controller) return;
    const QImage f = m_controller->frame();
    if (f.isNull()) return;
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->drawImage(boundingRect(), f); // o vídeo é pedido no tamanho da área: preenche
}
