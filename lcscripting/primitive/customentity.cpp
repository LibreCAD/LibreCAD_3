#include "customentity.h"
#include "../builders/customentity.h"

// Phase 6 PR-6.1 sub-piece 2b — dispatch via CustomEntityDispatchHook
// (installed by lcadluascript's LuaCustomEntityDispatchHook at LCLua
// construction time).  See lcscripting/customentitydispatchhook.h for
// the hook contract.
#include "../customentitydispatchhook.h"

using namespace lc;
using namespace entity;

namespace {

// Local helper: invoke a non-Lua callback through the neutral
// ScriptValue path.  Same pattern as sub-piece 2a's `invokeNonLua`
// which lived in lcadluascript pre-move; identical behavior.  Native
// callbacks fire with real side effects; ScriptValue kinds without
// corresponding types (Builder_SPtr, SimpleSnapConstrain) pass as
// Nil for now.
lc::scripting::ScriptValue invokeNonLua(
    const lc::scripting::ScriptCallback& callback,
    const std::vector<lc::scripting::ScriptValue>& args) {
    if (callback.isNil()) {
        return lc::scripting::ScriptValue{};
    }
    return callback.invoke(args);
}

} // namespace

ScriptCustomEntity::ScriptCustomEntity(const lc::builder::CustomEntityBuilder& builder) :
    CustomEntity(builder),
    _snapPoints(builder.snapFunction()),
    _nearestPoint(builder.nearestPointFunction()),
    _dragPoints(builder.dragPointsFunction()),
    _newDragPoint(builder.newDragPointFunction()),
    _dragPointClick(builder.dragPointsClickedFunction()),
    _dragPointRelease(builder.dragPointsReleasedFunction()) {
}

ScriptCustomEntity::ScriptCustomEntity(const Insert_CSPtr& insert,
                                       const ScriptCustomEntity_CSPtr& customEntity,
                                       bool sameID) :
    CustomEntity(insert, sameID),
    _snapPoints(customEntity->_snapPoints),
    _nearestPoint(customEntity->_nearestPoint),
    _dragPoints(customEntity->_dragPoints),
    _newDragPoint(customEntity->_newDragPoint),
    _dragPointClick(customEntity->_dragPointClick),
    _dragPointRelease(customEntity->_dragPointRelease) {
}

std::vector<EntityCoordinate> ScriptCustomEntity::snapPoints(const geo::Coordinate& coord,
        const SimpleSnapConstrain& simpleSnapConstrain,
        double minDistanceToSnap,
        int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;

    // Lua fast path via the installed dispatch hook.  If the hook
    // handles the callback (returns true), `points` is populated with
    // concrete Insert-typed userdata semantics preserved.
    auto hook = lc::scripting::customEntityDispatchHook();
    if (hook && hook->trySnapPoints(_snapPoints, shared_from_this(),
                                    coord, simpleSnapConstrain,
                                    minDistanceToSnap, maxNumberOfSnapPoints,
                                    points)) {
        Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
        return points;
    }

    // Non-Lua path: fire callback via neutral ScriptValue.  Return type
    // (vector<EntityCoordinate>) has no ScriptValue kind, so the
    // callback's return is IGNORED — dispatch is for side effects only
    // until ScriptValue grows an EntityCoordinateList kind.
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNonLua(_snapPoints, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue(coord),
        // SimpleSnapConstrain has no ScriptValue kind — Nil placeholder.
        lc::scripting::ScriptValue{},
        lc::scripting::ScriptValue(minDistanceToSnap),
        lc::scripting::ScriptValue(maxNumberOfSnapPoints),
    });
    return points;
}

geo::Coordinate ScriptCustomEntity::nearestPointOnPath(const geo::Coordinate& coord) const {
    geo::Coordinate result;

    auto hook = lc::scripting::customEntityDispatchHook();
    if (hook && hook->tryNearestPointOnPath(_nearestPoint, shared_from_this(),
                                            coord, result)) {
        return result;
    }

    // Non-Lua path: fire callback + extract Coordinate return if
    // present, else fall back to query point.
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    lc::scripting::ScriptValue ret = invokeNonLua(_nearestPoint, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue(coord),
    });
    if (ret.kind() == lc::scripting::ScriptValue::Kind::Coordinate) {
        return ret.asCoordinate();
    }
    return coord;
}

CADEntity_CSPtr ScriptCustomEntity::modifyInsert(const CADEntity_CSPtr& insert) const {
    auto i = std::dynamic_pointer_cast<const Insert>(insert);

    if(!i) {
        return shared_from_this();
    }

    return std::make_shared<ScriptCustomEntity>(
        i,
        std::static_pointer_cast<const ScriptCustomEntity>(shared_from_this()),
        true);
}

CADEntity_CSPtr ScriptCustomEntity::move(const geo::Coordinate& offset) const {
    return modifyInsert(Insert::move(offset));
}

CADEntity_CSPtr ScriptCustomEntity::copy(const geo::Coordinate& offset) const {
    return modifyInsert(Insert::copy(offset));
}

CADEntity_CSPtr ScriptCustomEntity::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    return modifyInsert(Insert::rotate(rotation_center, rotation_angle));
}

CADEntity_CSPtr ScriptCustomEntity::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    return modifyInsert(Insert::scale(scale_center, scale_factor));
}

CADEntity_CSPtr ScriptCustomEntity::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    return modifyInsert(Insert::mirror(axis1, axis2));
}

CADEntity_CSPtr ScriptCustomEntity::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    return modifyInsert(Insert::modify(layer, metaInfo, block));
}

std::map<unsigned int, geo::Coordinate> ScriptCustomEntity::dragPoints() const {
    std::map<unsigned int, geo::Coordinate> result;

    auto hook = lc::scripting::customEntityDispatchHook();
    if (hook && hook->tryDragPoints(_dragPoints, shared_from_this(), result)) {
        return result;
    }

    // Non-Lua path: fire for side effects; return type has no
    // ScriptValue kind, so the callback's return is discarded.
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNonLua(_dragPoints, { lc::scripting::ScriptValue(self) });
    return result;
}

void ScriptCustomEntity::onDragPointClick(lc::operation::Builder_SPtr builder, unsigned int point) const {
    auto hook = lc::scripting::customEntityDispatchHook();
    if (hook && hook->tryOnDragPointClick(_dragPointClick, shared_from_this(),
                                          builder, point)) {
        return;
    }

    // Non-Lua path.  Builder_SPtr passes as Nil (no ScriptValue kind).
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNonLua(_dragPointClick, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue{},  // Builder_SPtr — no kind yet
        lc::scripting::ScriptValue(static_cast<int>(point)),
    });
}

void ScriptCustomEntity::onDragPointRelease(lc::operation::Builder_SPtr builder) const {
    auto hook = lc::scripting::customEntityDispatchHook();
    if (hook && hook->tryOnDragPointRelease(_dragPointRelease, shared_from_this(),
                                            builder)) {
        return;
    }

    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNonLua(_dragPointRelease, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue{},  // Builder_SPtr — no kind yet
    });
}

void ScriptCustomEntity::setDragPoint(lc::geo::Coordinate position) const {
    auto hook = lc::scripting::customEntityDispatchHook();
    if (hook && hook->trySetDragPoint(_newDragPoint, shared_from_this(),
                                      position)) {
        return;
    }

    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNonLua(_newDragPoint, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue(position),
    });
}
