#include "scenemanager.h"

#include <QDebug>
#include <QGraphicsLineItem>

#include "graphicsitems/lccircleitem.h"
#include "graphicsitems/lcarcitem.h"
#include "graphicsitems/lclineitem.h"

SceneManager::SceneManager(LCADViewer* viewer, lc::AbstractDocument* document) : QObject() {
    _viewer = viewer;
    _document = document;
    connect(_document, SIGNAL(addEntityEvent(const lc::AddEntityEvent&)),
            this, SLOT(on_addEntityEvent(const lc::AddEntityEvent&)));
    connect(_document, SIGNAL(removeEntityEvent(const lc::RemoveEntityEvent&)),
            this, SLOT(on_removeEntityEvent(const lc::RemoveEntityEvent&)));
}


/**
  * Function to add entities to the graphics scene
  */
void SceneManager::on_addEntityEvent(const lc::AddEntityEvent& event) {
    QGraphicsScene* scene = _viewer->scene();

    // Add a line
    const lc::Line* line = dynamic_cast<const lc::Line*>(event.entity().get());

    if (line != NULL) {
        // This might be slow on clang, I have no idea but it's performs a lot worse
        LCLineItem* foo = new LCLineItem(line);
        foo->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(line->id(), foo);
        return;
    }

    // Add a circle
    const lc::Circle* circle = dynamic_cast<const lc::Circle*>(event.entity().get());

    if (circle != NULL) {
        LCCircleItem* foo = new LCCircleItem(circle);
        foo->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(circle->id(), foo);
        return;
    }

    // Add a Arc
    const lc::Arc* arc = dynamic_cast<const lc::Arc*>(event.entity().get());

    if (arc != NULL) {
        LCArcItem* foo = new LCArcItem(arc);
        foo->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(arc->id(), foo);
        return;
    }
}

/**
  * Function to remove a entity from the graphics scene on request
  */
void SceneManager::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {
    QGraphicsItem* item = _activeGraphicsItems.value(event.id());
    _activeGraphicsItems.remove(event.id());
    _viewer->scene()->removeItem(item);

}
