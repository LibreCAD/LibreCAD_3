#include "snapmanagerimpl.h"
#include <cad/vo/entitycoordinate.h>
#include <cad/primitive/circle.h>

SnapManagerImpl::SnapManagerImpl(DocumentCanvas_SPtr view, lc::Snapable_CSPtr grid, double distanceToSnap)  :  _grid(grid), _distanceToSnap(distanceToSnap), _view(view) {

/*
    connect(view, SIGNAL(mouseMoveEvent(const MouseMoveEvent&)),
            this, SLOT(on_mouseMoveEvent(const MouseMoveEvent&)));

    connect(view, SIGNAL(mouseReleaseEvent(const MouseReleaseEvent&)),
            this, SLOT(on_mouseRelease_Event(const MouseReleaseEvent&)));
*/
}

/**
 * I am considering to make this function such that the order of testing ofr snap point's
 * are done based on some functor where we can change the order.
 */
void SnapManagerImpl::setDeviceLocation(int x, int y) {
    double x_ = x;
    double y_ = y;

    _view->device_to_user(&x_, &y_);
    auto location = lc::geo::Coordinate(x_, y_);


    // Calculate from the number of pixels the distance  for snapping
    // For best results use a un-even number of pixels
    double zeroCornerX = 0.;
    double zeroCornerY = 0.;
    _view->device_to_user(&zeroCornerX, &zeroCornerY);

    double gridSPacingX = _distanceToSnap;
    double gridSPacingY = _distanceToSnap;
    _view->device_to_user(&gridSPacingX, &gridSPacingY);

    double realDistanceForPixels = (gridSPacingX - zeroCornerX) / 2.0;


    // We should call this function only if the mouse haven't moved for XX milli seconds

    // Find all entities that are close to the current mouse pointer

    std::vector<lc::EntityDistance> entities = _view->entityContainer().getEntitiesNearCoordinate(location, realDistanceForPixels);

    if (entities.size() > 0) {
        // auto item = entities.begin();
        // auto event = SnapPointEvent(item->coordinate());
        // _snapPointEvent(event);
        // std::cerr << "Found " << entities.size() << " entities close to the cursor" << location.x() << ":" << location.y() << "\n";
    }

    // Emit Snappoint event if a entity intersects with a other entity
    // TODO: Need some modification to find the closest intersection point
    if (entities.size() > 1) {
        std::sort(entities.begin(), entities.end(), lc::EntityDistanceSorter(location));

        for (size_t a = 0; a < entities.size(); a++) {
            for (size_t b = a + 1; b < entities.size(); b++) {
                lc::entity::CADEntity_CSPtr i1 = entities.at(a).entity();
                lc::entity::CADEntity_CSPtr i2 = entities.at(b).entity();
                lc::Intersect intersect(lc::Intersect::OnPath, LCTOLERANCE);



                if (intersect.result().size() > 0) {
                    std::vector<lc::geo::Coordinate> coords = intersect.result();
                    std::sort(coords.begin(), coords.end(), lc::geo::CoordinateDistanceSort(location));
                    lc::geo::Coordinate sp = coords.at(0);

                    if ((location - sp).magnitude() < realDistanceForPixels) {
                        auto item = entities.begin();
                        auto event = SnapPointEvent(sp);
                        _snapPointEvent(event);
                        return;
                    }
                }
            }
        }
    }
    /*

    // Emit snappoint based on closest entity
    if (_entities.size() > 0) {
        // Get the snap point that is closest to the mouse pointer from all entities
        qSort(_entities.begin(), _entities.end(), lc::EntityDistance::sortAscending);
        const lc::Snapable_CSPtr captr = std::dynamic_pointer_cast<const lc::Snapable>(_entities.at(0).entity());
        // TODO: Decide how to handle maximum number of snap points, and how we are going to return specific snappoints like centers + near
        std::vector<lc::EntityCoordinate> sp = captr->snapPoints(event.mousePosition(), realDistanceForPixels, 10);
        SnapPointEvent snapEvent(sp.at(0).coordinate());
        _lastSnapEvent = snapEvent;
        qDebug() << "Snap to entity";
        emit snapPointEvent(snapEvent);
        return;
    }
    */

    // If no entity was found to snap against, then snap to grid
    if (_gridSnappable == true) {
        std::vector<lc::EntityCoordinate> points = _grid->snapPoints(location, _snapConstrain, realDistanceForPixels, 1);
        std::cerr << "_gridSnappable" << _gridSnappable << "\n";
        if (points.size() > 0) {
            auto item = entities.begin();
            auto event = SnapPointEvent(points.at(0).coordinate());
            _snapPointEvent(event);
            return;
        }
    }

    // FIXME: Currently sending a snapEvent so the cursor get's updated, what we really want is some sort of a release snap event
    // but only when we had a snap, but just lost it
    //SnapPointEvent snapEvent(lc::geo::Coordinate(event.mousePosition().x(), event.mousePosition().y()));
    //_lastSnapEvent = snapEvent;
    //qDebug() << "Snap to Free";
    //emit snapPointEvent(snapEvent);
}


void SnapManagerImpl::setGridSnappable(bool gridSnappable) {
    _gridSnappable = gridSnappable;
}

bool SnapManagerImpl::isGridSnappable() const {
    //return _gridSnappable;
}

Nano::Signal<void(const SnapPointEvent&)> &SnapManagerImpl::snapPointEvents() {
    return _snapPointEvent;
}
lc::SimpleSnapConstrain SnapManagerImpl::snapConstrain() const {
    return _snapConstrain;
}

void SnapManagerImpl::snapConstrain(const lc::SimpleSnapConstrain & snapConstrain) {
    _snapConstrain = snapConstrain;
}