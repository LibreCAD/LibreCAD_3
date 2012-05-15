#include "snapmanagerimpl.h"
#include "cad/functions/intersect.h"

SnapManagerImpl::SnapManagerImpl(LCADViewer* graphicsView, std::tr1::shared_ptr<lc::SelectionManager> selectionmanager, std::tr1::shared_ptr<const lc::Snapable> grid, double distanceToSnap)  : _selectionmanager(selectionmanager), _grid(grid), _distanceToSnap(distanceToSnap) {

    connect(graphicsView, SIGNAL(mouseMoveEvent(const MouseMoveEvent&)),
            this, SLOT(on_mouseMoveEvent(const MouseMoveEvent&)));

    connect(graphicsView, SIGNAL(mouseReleaseEvent(const MouseReleaseEvent&)),
            this, SLOT(on_mouseRelease_Event(const MouseReleaseEvent&)));
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

    qDebug() << "c:" << entities.count();

    // Emit Snappoint event if a entity intersects with a other entity an is s
    if (entities.count() > 1) {
        for (int a = 0; a < entities.count(); a++) {
            for (int b = a + 1; b < entities.count(); b++) {


                std::tr1::shared_ptr<const lc::CADEntity> i1 = entities.at(a).entity();
                std::tr1::shared_ptr<const lc::CADEntity> i2 = entities.at(b).entity();

                lc::Intersect intersect(lc::Intersect::MustIntersect);
                i1->accept(i2, intersect);

                if (intersect.result().count() > 0) {
                    QList<lc::geo::Coordinate> coords = intersect.result();
                    qSort(coords.begin(), coords.end(), lc::geo::CoordinateDistanceSort(event.mousePosition()));
                    lc::geo::Coordinate sp = coords.at(0);

                    if ((event.mousePosition() - sp).magnitude() < realDistanceForPixels) {
                        SnapPointEvent snapEvent(sp);
                        _lastSnapEvent = snapEvent;
                        emit snapPointEvent(snapEvent);
                        return;
                    }
                }
            }
        }
    }

    // Eimit snappoint based on closest entity
    if (entities.count() > 0) {
        // Get the snap point that is closest to the mouse pointer from all entities
        qSort(entities.begin(), entities.end(), lc::EntityDistance::sortAscending);
        const std::tr1::shared_ptr<const lc::Snapable> captr = std::tr1::dynamic_pointer_cast<const lc::Snapable>(entities.at(0).entity());
        // TODO: Decide how to handle maximum number of snap points, and how we are going to return specific snappoints like centers + near
        QList<lc::EntityCoordinate> sp = captr->snapPoints(event.mousePosition(), realDistanceForPixels, 10);
        SnapPointEvent snapEvent(sp.at(0).coordinate());
        _lastSnapEvent = snapEvent;
        emit snapPointEvent(snapEvent);
        return;
    }

    // If no entity was found to snap against, then snap to grid
    QList<lc::EntityCoordinate> points = _grid->snapPoints(lc::geo::Coordinate(event.mousePosition().x(), event.mousePosition().y()), realDistanceForPixels, 1);

    if (points.length() > 0) {
        SnapPointEvent snapEvent(points.at(0).coordinate());
        _lastSnapEvent = snapEvent;
        emit snapPointEvent(snapEvent);
        return;
    }

    // FIXME: Currently sending a snapEvent so the cursor get's updated, what we really want is some sort of a release snap event
    // but only when we had a snap, but just lost it
    SnapPointEvent snapEvent;
    _lastSnapEvent = snapEvent;
    emit snapPointEvent(snapEvent);
}


void SnapManagerImpl::on_mouseRelease_Event(const MouseReleaseEvent& event) {
    if (_lastSnapEvent.status() == true) {
        MouseReleaseEvent snappedLocation(event.view(), _lastSnapEvent.snapPoint());
        emit mouseReleaseEvent(snappedLocation);
    } else {
        emit mouseReleaseEvent(event);
    }
}
