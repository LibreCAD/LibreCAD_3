#include "scenemanager.h"

#include <QDebug>
#include <QGraphicsLineItem>
#include "cad/base/cadentity.h"
#include "cad/primitive/line.h"
#include "cad/primitive/Circle.h"

SceneManager::SceneManager(LCADViewer* viewer, lc::AbstractDocument* document) : QObject() {
    _viewer = viewer;
    _document = document;
    connect(_document, SIGNAL(addEntityEvent(lc::AddEntityEvent*)),
            this, SLOT(on_addEntityEvent(lc::AddEntityEvent*)));
}


void SceneManager::on_addEntityEvent(lc::AddEntityEvent* event) {
    QGraphicsScene* scene = _viewer->scene();


    // Add a line
    lc::Line* l = dynamic_cast<lc::Line*>(event->entity());

    if (l != NULL) {
        QGraphicsLineItem* foo = scene->addLine(l->start().x(), l->start().y(), l->end().x(), l->end().y());
        foo->setFlag(QGraphicsItem::ItemIsMovable);
        return;
    }

    // Add a circle
    lc::Circle* c = dynamic_cast<lc::Circle*>(event->entity());

    if (c != NULL) {
        QGraphicsEllipseItem* foo = scene->addEllipse(c->center().x(), c->center().y(), c->radius(), c->radius());
        foo->setFlag(QGraphicsItem::ItemIsMovable);
        return;
    }
}


