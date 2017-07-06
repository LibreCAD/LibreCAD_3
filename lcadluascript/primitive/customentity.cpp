#include <builders/customentity.h>
#include "customentity.h"

using namespace lc;
using namespace entity;

LuaCustomEntity::LuaCustomEntity(const lc::builder::CustomEntityBuilder& builder) :
        CustomEntity(builder),
        _snapPoints(builder.snapFunction()),
        _nearestPoint(builder.nearestPointFunction()) {
}

std::vector<EntityCoordinate> LuaCustomEntity::snapPoints(const geo::Coordinate& coord,
                                                       const SimpleSnapConstrain& simpleSnapConstrain,
                                                       double minDistanceToSnap,
                                                       int maxNumberOfSnapPoints) const {
    auto snapPointsDupl = _snapPoints; //Dirty hack to call Lua function from a const function
    auto points = snapPointsDupl.call<std::vector<EntityCoordinate>>(coord, simpleSnapConstrain, minDistanceToSnap, maxNumberOfSnapPoints);
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);

    return points;
}

geo::Coordinate LuaCustomEntity::nearestPointOnPath(const geo::Coordinate& coord) const {
    auto nearestPointDupl = _nearestPoint;
    return nearestPointDupl.call<geo::Coordinate>(coord);
}
