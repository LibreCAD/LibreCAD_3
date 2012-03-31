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
    connect(_document, SIGNAL(removeEntityEvent(lc::RemoveEntityEvent*)),
            this, SLOT(on_removeEntityEvent(lc::RemoveEntityEvent*)));
}


void SceneManager::on_addEntityEvent(lc::AddEntityEvent* event) {
    QGraphicsScene* scene = _viewer->scene();


    // Add a line
    lc::Line* l = dynamic_cast<lc::Line*>(event->entity());

    if (l != NULL) {
        QGraphicsLineItem* foo = scene->addLine(l->start().x(), l->start().y(), l->end().x(), l->end().y());
        foo->setFlag(QGraphicsItem::ItemIsMovable);
        foo->setData(0, QVariant((int)l->id()));
        return;
    }

    // Add a circle
    lc::Circle* c = dynamic_cast<lc::Circle*>(event->entity());

    if (c != NULL) {
        QGraphicsEllipseItem* foo = scene->addEllipse(c->center().x(), c->center().y(), c->radius(), c->radius());
        foo->setFlag(QGraphicsItem::ItemIsMovable);
        foo->setData(0, QVariant((int)c->id()));
        return;
    }
}


void SceneManager::on_removeEntityEvent(lc::RemoveEntityEvent* event) {
    QGraphicsScene* scene = _viewer->scene();

    QList<QGraphicsItem*>items =    scene->items();

    for (int i = 0;  i < items.size(); i++) {
        QGraphicsItem* item = items.at(i);

        if (item->data(0) == (int)event->id()) {
            scene->removeItem(item);
        }
    }

}
