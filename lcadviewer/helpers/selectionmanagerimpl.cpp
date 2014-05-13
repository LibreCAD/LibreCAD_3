#include "selectionmanagerimpl.h"

#include "qdebug.h"
#include <QGraphicsItem>
#include "cad/interface/snapable.h"
#include "cad/vo/entitydistance.h"
#include "cad/functions/intersect.h"

SelectionManagerImpl::SelectionManagerImpl(std::shared_ptr<lc::StorageManager> storageManager,  LCADViewer* view) : lc::SelectionManager(),  _storageManager(storageManager), _view(view) {
}


QList<lc::EntityDistance> SelectionManagerImpl::getEntitiesNearCoordinate(const lc::geo::Coordinate& point, double distance) const {

    // This routine gives us a rought estimate of all items around the point
    // this is done by checking the bounding rect. It's also fast because it uses the BSD tree from the scene

    /*
    QList<QGraphicsItem*> items = _view->scene()->items(
                                      point.x() - distance,
                                      point.y() - distance,
                                      distance * 2.0,
                                      distance * 2.0,
                                      Qt::IntersectsItemBoundingRect,
                                      Qt::AscendingOrder); */

    QList<lc::EntityDistance> entities;
    /*


        // Now calculate for each entity if we are near the entities path

        for (int i = 0; i < items.count(); i++) {
            LCGraphicsItem* item = dynamic_cast<LCGraphicsItem*>(items.at(i));

            // If item == NULL then this item was not  a type of LCGraphicsItem, so no bother to test it further
            if (item != NULL) {
                std::shared_ptr<const lc::Snapable> entity = std::dynamic_pointer_cast<const lc::Snapable>(item->entity());

                if (entity != NULL) { // Not all entities might be snapable, so we only test if this is possible.
                    lc::geo::Coordinate eCoordinate = entity->nearestPointOnPath(point);
                    lc::geo::Coordinate nearestCoord = eCoordinate - point;

                    double cDistance = nearestCoord.magnitude();

                    if (cDistance < distance) {
                        entities.append(lc::EntityDistance(item->entity(), cDistance));
                    }
                }
            }
        }
    */
    return entities;
}


QList<std::shared_ptr<const lc::CADEntity> > SelectionManagerImpl::getEntitiesInArea(const lc::geo::Area& area) const {
    // QList<LCGraphicsItem*> items = _view->items();
    // QList<lc::EntityDistance> entities;

    //for (int i = 0; i < items.count(); i++) {
    //   entities.append(items.at(i)->entity());
    //}


}

