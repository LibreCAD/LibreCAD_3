#include "snapmanagerimpl.h"

SnapManagerImpl::SnapManagerImpl(LCADViewer* viewer, lc::SelectionManagerPtr selectionmanager, lc::SnapablePtr grid, double distanceToSnap)  : _selectionmanager(selectionmanager), _grid(grid), _distanceToSnap(distanceToSnap) {

    connect(viewer, SIGNAL(mouseMoveEvent(const MouseMoveEvent&)),
            this, SLOT(on_mouseMoveEvent(const MouseMoveEvent&)));
}


void SnapManagerImpl::on_mouseMoveEvent(const MouseMoveEvent& event) {

    // Calculate from the number of pixels the distance  for snapping
    // For best results use a un-even number of pixels
    QPointF zeroCorner = event.view()->mapToScene(0, 0);
    QPointF minGridSpaceCorner = event.view()->mapToScene(_distanceToSnap, 0);
    double realDistanceForPixels = minGridSpaceCorner.x() - zeroCorner.x();


    // We should call this function only if the mouse haven't moved for XX milli seconds

    // Find all entities that are close to the current mouse pointer
    QList<lc::EntityDistance> entities = _selectionmanager->getEntitiesNearCoordinate(event.mousePosition(), realDistanceForPixels);

    if (entities.count() > 0) {
        lc::EntityDistance ed = entities.at(0);
        double closestDistance = ed.distance();

        // Locate the entity that is closest to the mousepointer
        for (int i = 1; i < entities.count(); i++) {
            if (entities.at(i).distance() < closestDistance) {
                closestDistance = entities.at(i).distance();
                ed = entities.at(0);
            }
        }

        // Get the snap point that is closest to the mouse pointer from all entities
        const lc::SnapablePtr captr = dynamic_pointer_cast<const lc::Snapable>(ed.entity());
        QList<lc::EntityCoordinate> sp = captr->snapPoints(event.mousePosition(), realDistanceForPixels, 1);
        SnapPointEvent snapEvent(sp.at(0).coordinate());
        emit snapPointEvent(snapEvent);
        return;
    }

    // If no entity was found to snap against, then snap to grid
    QList<lc::EntityCoordinate> points = _grid->snapPoints(lc::geo::Coordinate(event.mousePosition().x(), event.mousePosition().y()), realDistanceForPixels, 1);

    if (points.length() > 0) {
        SnapPointEvent snapEvent(points.at(0).coordinate());
        emit snapPointEvent(snapEvent);
    }
}
