#include "scenemanager.h"

#include <QDebug>
#include <QGraphicsLineItem>

#include "cad/primitive/line.h"
#include "cad/primitive/Circle.h"

SceneManager::SceneManager(LCADViewer* viewer, lc::AbstractDocument* document) : QObject() {
    _viewer = viewer;
    _document = document;
    connect(_document, SIGNAL(addEntityEvent(const lc::AddEntityEvent&)),
            this, SLOT(on_addEntityEvent(const lc::AddEntityEvent&)));
    connect(_document, SIGNAL(removeEntityEvent(const lc::RemoveEntityEvent&)),
            this, SLOT(on_removeEntityEvent(const lc::RemoveEntityEvent&)));
}


void SceneManager::on_addEntityEvent(const lc::AddEntityEvent& event) {
    QGraphicsScene* scene = _viewer->scene();

    // Add a line
    lc::Line* l = dynamic_cast<lc::Line*>(event.entity().get());

    if (l != NULL) {
        // This might be slow on clang, I have no idea but it's performs a lot worse
        QGraphicsLineItem* foo = scene->addLine(l->start().x(), l->start().y(), l->end().x(), l->end().y());
        foo->setFlag(QGraphicsItem::ItemIsMovable);
        _activeGraphicsItems.insert(l->id(), foo);
        return;
    }

    // Add a circle
    lc::Circle* c = dynamic_cast<lc::Circle*>(event.entity().get());

    if (c != NULL) {
        QGraphicsEllipseItem* foo = scene->addEllipse(c->center().x(), c->center().y(), c->radius(), c->radius());
        foo->setFlag(QGraphicsItem::ItemIsMovable);
        foo->setData(0, QVariant((int)c->id()));
        _activeGraphicsItems.insert(event.entity()->id(), foo);
        return;
    }
}


void SceneManager::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {
    QGraphicsItem* item = _activeGraphicsItems.value(event.id());
    _activeGraphicsItems.remove(event.id());
    _viewer->scene()->removeItem(item);

}
