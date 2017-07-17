#include <builders/customentity.h>
#include "customentity.h"

using namespace lc;
using namespace entity;

LuaCustomEntity::LuaCustomEntity(const lc::builder::CustomEntityBuilder& builder) :
        CustomEntity(builder),
        _snapPoints(builder.snapFunction()),
        _nearestPoint(builder.nearestPointFunction()),
        _dragPoints(builder.dragPointsFunction()),
        _newDragPoints(builder.newDragPointsFunction()) {
}

LuaCustomEntity::LuaCustomEntity(Insert_CSPtr insert, LuaCustomEntity_CSPtr customEntity, bool sameID) :
    CustomEntity(insert, sameID),
    _snapPoints(customEntity->_snapPoints),
    _nearestPoint(customEntity->_nearestPoint),
    _dragPoints(customEntity->_dragPoints),
    _newDragPoints(customEntity->_newDragPoints) {

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

CADEntity_CSPtr LuaCustomEntity::modifyInsert(CADEntity_CSPtr insert) const {
    auto i = std::dynamic_pointer_cast<const Insert>(insert);

    if(!i) {
        return shared_from_this();
    }

    return std::make_shared<LuaCustomEntity>(i, std::static_pointer_cast<const LuaCustomEntity>(shared_from_this()), true);
}

CADEntity_CSPtr LuaCustomEntity::move(const geo::Coordinate& offset) const {
    return modifyInsert(Insert::move(offset));
}

CADEntity_CSPtr LuaCustomEntity::copy(const geo::Coordinate& offset) const {
    return modifyInsert(Insert::copy(offset));
}

CADEntity_CSPtr LuaCustomEntity::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    return modifyInsert(Insert::rotate(rotation_center, rotation_angle));
}

CADEntity_CSPtr LuaCustomEntity::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    return modifyInsert(Insert::scale(scale_center, scale_factor));
}

CADEntity_CSPtr LuaCustomEntity::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    return modifyInsert(Insert::mirror(axis1, axis2));
}

CADEntity_CSPtr LuaCustomEntity::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    return modifyInsert(Insert::modify(layer, metaInfo));
}

std::map<unsigned int, geo::Coordinate> LuaCustomEntity::dragPoints() const {
    auto dragPointsDupl = _dragPoints;
    return dragPointsDupl.call<std::map<unsigned int, geo::Coordinate>>();
}

CADEntity_CSPtr LuaCustomEntity::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    auto newDragPointsDupl = _newDragPoints;
    return newDragPointsDupl.call<CADEntity_CSPtr>(dragPoints);
}
