#pragma once

#include "projection/ProjectionController.h"

#include <QPointer>
#include <QQuickPaintedItem>

/** Mostra a tela do celular (QML: `ProjectionView { controller: projection }`). */
class ProjectionView : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(ProjectionController *controller READ controller WRITE setController NOTIFY controllerChanged)
public:
    explicit ProjectionView(QQuickItem *parent = nullptr) : QQuickPaintedItem(parent) {}

    ProjectionController *controller() const { return m_controller; }
    void setController(ProjectionController *c)
    {
        if (c == m_controller) return;
        if (m_controller) disconnect(m_controller, nullptr, this, nullptr);
        m_controller = c;
        if (c) connect(c, &ProjectionController::frameChanged, this, [this] { update(); });
        emit controllerChanged();
        update();
    }

    void paint(QPainter *p) override;

signals:
    void controllerChanged();

private:
    QPointer<ProjectionController> m_controller;
};
