#include "snapmanagerimpl.h"
#include "cad/functions/intersect.h"
#include "cad/vo/entitydistance.h"

SnapManagerImpl::SnapManagerImpl(LCADViewer* view, lc::Snapable_CSPtr grid, double distanceToSnap)  :  _grid(grid), _distanceToSnap(distanceToSnap) {

    connect(view, SIGNAL(mouseMoveEvent(const MouseMoveEvent&)),
            this, SLOT(on_mouseMoveEvent(const MouseMoveEvent&)));

    connect(view, SIGNAL(mouseReleaseEvent(const MouseReleaseEvent&)),
            this, SLOT(on_mouseRelease_Event(const MouseReleaseEvent&)));

}


void SnapManagerImpl::on_mouseMoveEvent(const MouseMoveEvent& event) {

    return;

    // Calculate from the number of pixels the distance  for snapping
    // For best results use a un-even number of pixels
    double zeroCornerX = 0.;
    double zeroCornerY = 0.;
    event.painter()->device_to_user(&zeroCornerX, &zeroCornerY);

    double gridSPacingX = _distanceToSnap;
    double gridSPacingY = _distanceToSnap;
    event.painter()->device_to_user(&gridSPacingX, &gridSPacingY);

    double realDistanceForPixels = (gridSPacingX - zeroCornerX) / 2.0;


    // We should call this function only if the mouse haven't moved for XX milli seconds

    // Find all entities that are close to the current mouse pointer
    _entities = _selectionmanager->getEntitiesNearCoordinate(event.mousePosition(), realDistanceForPixels);

    if (_entities.count() > 0) {
        qDebug() << "Found" << _entities.count() << "entities close to the cursor";
    }

    // Emit Snappoint event if a entity intersects with a other entity
    // TODO: Need some modification to find the closest intersection point
    if (_entities.count() > 1) {
        qSort(_entities.begin() , _entities.end(), lc::EntityDistance::sortAscending);

        for (int a = 0; a < _entities.count(); a++) {
            for (int b = a + 1; b < _entities.count(); b++) {
                lc::CADEntity_CSPtr i1 = _entities.at(a).entity();
                lc::CADEntity_CSPtr i2 = _entities.at(b).entity();

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

    // Emit snappoint based on closest entity
    if (_entities.count() > 0) {
        // Get the snap point that is closest to the mouse pointer from all entities
        qSort(_entities.begin(), _entities.end(), lc::EntityDistance::sortAscending);
        const lc::Snapable_CSPtr captr = std::dynamic_pointer_cast<const lc::Snapable>(_entities.at(0).entity());
        // TODO: Decide how to handle maximum number of snap points, and how we are going to return specific snappoints like centers + near
        QList<lc::EntityCoordinate> sp = captr->snapPoints(event.mousePosition(), realDistanceForPixels, 10);
        SnapPointEvent snapEvent(sp.at(0).coordinate());
        _lastSnapEvent = snapEvent;
        qDebug() << "Snap to entity";
        emit snapPointEvent(snapEvent);
        return;
    }

    // If no entity was found to snap against, then snap to grid
    if (_gridSnappable == true) {
        QList<lc::EntityCoordinate> points = _grid->snapPoints(lc::geo::Coordinate(event.mousePosition().x(), event.mousePosition().y()), realDistanceForPixels, 1);

        if (points.length() > 0) {
            SnapPointEvent snapEvent(points.at(0).coordinate());
            _lastSnapEvent = snapEvent;
            qDebug() << "Snap to Grid";
            emit snapPointEvent(snapEvent);
            return;
        }
    }

    // FIXME: Currently sending a snapEvent so the cursor get's updated, what we really want is some sort of a release snap event
    // but only when we had a snap, but just lost it
    SnapPointEvent snapEvent(lc::geo::Coordinate(event.mousePosition().x(), event.mousePosition().y()));
    _lastSnapEvent = snapEvent;
    qDebug() << "Snap to Free";
    emit snapPointEvent(snapEvent);

}


void SnapManagerImpl::on_mouseRelease_Event(const MouseReleaseEvent& event) {
    /*
    if (_lastSnapEvent.status() == true) {
        MouseReleaseEvent snappedLocation(_lastSnapEvent.snapPoint(), _entities);

        if (event.mouseEvent()->button() & Qt::RightButton) {
            emit mouseRightReleaseEvent(snappedLocation);
        } else {
            emit mouseReleaseEvent(snappedLocation);
        }
    } else {
        if (event.mouseEvent()->button() & Qt::RightButton) {
            emit mouseRightReleaseEvent(event);
        } else {
            emit mouseReleaseEvent(event);
        }
    }
    */
}


void SnapManagerImpl::setGridSnappable(bool gridSnappable) {
    _gridSnappable = gridSnappable;
}

bool SnapManagerImpl::isGridSnappable() const {
    return _gridSnappable;
}
