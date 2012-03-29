#include "scenemanager.h"

#include <QDebug>
#include <QGraphicsLineItem>
#include "cad/base/cadentity.h"
#include "cad/primitive/line.h"

SceneManager::SceneManager(LCADViewer* viewer, lc::AbstractDocument* document) : QObject() {
    _viewer = viewer;
    _document = document;
    connect(_document, SIGNAL(addEntityEvent(lc::AddEntityEvent*)),
            this, SLOT(on_addEntityEvent(lc::AddEntityEvent*)));
}


void SceneManager::on_addEntityEvent(lc::AddEntityEvent* event) {
    QGraphicsScene* scene = _viewer->scene();
    lc::Line* l = dynamic_cast<lc::Line*>(event->entity());
    QGraphicsLineItem* foo=scene->addLine(l->start().x(), l->start().y(), l->end().x(), l->end().y());
    foo->setFlag(QGraphicsItem::ItemIsMovable);
}


