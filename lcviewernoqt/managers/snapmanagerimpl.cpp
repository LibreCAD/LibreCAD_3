#include "snapmanagerimpl.h"
#include <cad/vo/entitycoordinate.h>
#include <cad/primitive/circle.h>
#include <cad/base/visitor.h>
#include <cad/base/cadentity.h>
#include <cad/functions/intersect.h>

SnapManagerImpl::SnapManagerImpl(DocumentCanvas_SPtr view, lc::Snapable_CSPtr grid, double distanceToSnap) : _grid(
        grid), _gridSnappable(false), _snapIntersections(false), _distanceToSnap(distanceToSnap), _view(view) {


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
    // TODO: We properly need to remember entities which we had snap point in the past, some sort og queue
    // this way we can get more efficiently snap to entities outside the cursor's 'range' so the
    // person can 'pick' a entity onceand then it would stay in the list of entities to
    // consider for snapping. THis will mostly lickly be lines only
    std::vector<lc::EntityDistance> entities = _view->entityContainer().getEntityPathsNearCoordinate(location,
                                                                                                  realDistanceForPixels);
    std::sort(entities.begin(), entities.end(), lc::EntityDistanceSorter(location));

    // Emit Snappoint event if a entity intersects with a other entity
    if (entities.size() > 1 && _snapIntersections) {

        for (size_t a = 0; a < entities.size(); a++) {
            for (size_t b = a + 1; b < entities.size(); b++) {
                lc::entity::CADEntity_CSPtr i1 = entities.at(a).entity();
                lc::entity::CADEntity_CSPtr i2 = entities.at(b).entity();

                lc::Intersect intersect(lc::Intersect::OnEntity, LCTOLERANCE);
                visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, *i1.get(), *i2.get());

                if (intersect.result().size() > 0) {
                    std::vector<lc::geo::Coordinate> coords = intersect.result();
                    std::sort(coords.begin(), coords.end(), lc::geo::CoordinateDistanceSort(location));

                    lc::geo::Coordinate sp = coords.at(0);
                    if ((location - sp).magnitude() < realDistanceForPixels) {
                        auto event = SnapPointEvent(sp);
                        _snapPointEvent(event);
                        return;
                    }
                }
            }
        }
    }

    // Emit snappoint based on closest entity
    if (entities.size() > 0) {
        // GO over all entities, first closest to teh cursor gradually moving away
        for (auto &entity : entities) {
            const lc::Snapable_CSPtr captr = std::dynamic_pointer_cast<const lc::Snapable>(entity.entity());
            if (captr) {
                // Locale snap points
                std::vector<lc::EntityCoordinate> sp = captr->snapPoints(location, _snapConstrain,
                                                                         realDistanceForPixels, 10);
                // When a snappoint was found, emit it
                if (sp.size() > 0) {
                    SnapPointEvent snapEvent(sp.at(0).coordinate());
                    _lastSnapEvent = snapEvent;
                    auto event = SnapPointEvent(sp.at(0).coordinate());
                    _snapPointEvent(event);
                    return;
                }
            }
        }
    }

    // If no entity was found to snap against, then snap to grid
    if (_gridSnappable == true) {
        std::vector<lc::EntityCoordinate> points = _grid->snapPoints(location, _snapConstrain, realDistanceForPixels,
                                                                     1);
        if (points.size() > 0) {
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


void SnapManagerImpl::setGridSnappable(bool enabled) {
    _gridSnappable = enabled;
}

bool SnapManagerImpl::isGridSnappable() const {
    return _gridSnappable;
}

void SnapManagerImpl::snapIntersections(bool enabled) {
    _snapIntersections = enabled;
}

bool SnapManagerImpl::snapIntersections() const {
    return _snapIntersections;
}


Nano::Signal<void(const SnapPointEvent &)> &SnapManagerImpl::snapPointEvents() {
    return _snapPointEvent;
}

lc::SimpleSnapConstrain SnapManagerImpl::snapConstrain() const {
    return _snapConstrain;
}

void SnapManagerImpl::snapConstrain(const lc::SimpleSnapConstrain &snapConstrain) {
    _snapConstrain = snapConstrain;
}