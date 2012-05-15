#include "scenemanager.h"

#include <QDebug>

#include "graphicsitems/lccircleitem.h"
#include "graphicsitems/lcarcitem.h"
#include "graphicsitems/lclineitem.h"
#include "graphicsitems/lcellipseitem.h"

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
    const std::tr1::shared_ptr<const lc::Line> line = std::tr1::dynamic_pointer_cast<const lc::Line>(event.entity());

    if (line != NULL) {
        LCLineItem* foo = new LCLineItem(line);
        //foo->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(line->id(), foo);
        return;
    }

    // Add a circle
    const std::tr1::shared_ptr<const lc::Circle> circle = std::tr1::dynamic_pointer_cast<const lc::Circle>(event.entity());

    if (circle != NULL) {
        LCCircleItem* foo = new LCCircleItem(circle);
        //foo->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(circle->id(), foo);
        return;
    }

    // Add a Arc
    const std::tr1::shared_ptr<const lc::Arc> arc = std::tr1::dynamic_pointer_cast<const lc::Arc>(event.entity());

    if (arc != NULL) {
        LCArcItem* foo = new LCArcItem(arc);
        //foo->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(arc->id(), foo);
        return;
    }

    // Add Ellipse
    const std::tr1::shared_ptr<const lc::Ellipse> ellipse = std::tr1::dynamic_pointer_cast<const lc::Ellipse>(event.entity());

    if (ellipse != NULL) {
        LCEllipseItem* foo = new LCEllipseItem(ellipse);
        //foo->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(ellipse->id(), foo);
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
