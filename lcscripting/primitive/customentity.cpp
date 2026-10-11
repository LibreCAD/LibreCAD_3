#include "customentity.h"
#include "../builders/customentity.h"

// Dispatch via the runtime-specific CustomEntityDispatchHook. Language
// adapters install their own hooks; one runtime cannot replace another.
#include "../customentitydispatchhook.h"

using namespace lc;
using namespace entity;

namespace {

// Local helper for the neutral ScriptValue fallback. Language callbacks
// should use their runtime-specific typed hook; native callbacks can use
// this path for values represented by ScriptValue.
lc::scripting::ScriptValue invokeNeutralCallback(
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

    // Typed adapter path preserves concrete Insert identity and the
    // EntityCoordinate result vector (which ScriptValue does not represent).
    auto hook = lc::scripting::customEntityDispatchHook(_snapPoints);
    if (hook && hook->trySnapPoints(_snapPoints, shared_from_this(),
                                    coord, simpleSnapConstrain,
                                    minDistanceToSnap, maxNumberOfSnapPoints,
                                    points)) {
        Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
        return points;
    }

    // Neutral fallback: snap vectors are not represented by ScriptValue,
    // so only a runtime-specific hook can provide snap results.
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNeutralCallback(_snapPoints, {
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

    auto hook = lc::scripting::customEntityDispatchHook(_nearestPoint);
    if (hook && hook->tryNearestPointOnPath(_nearestPoint, shared_from_this(),
                                            coord, result)) {
        return result;
    }

    // Neutral fallback: extract a Coordinate result if available, otherwise
    // keep the query point as the safe default.
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    lc::scripting::ScriptValue ret = invokeNeutralCallback(_nearestPoint, {
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

    auto hook = lc::scripting::customEntityDispatchHook(_dragPoints);
    if (hook && hook->tryDragPoints(_dragPoints, shared_from_this(), result)) {
        return result;
    }

    // Neutral fallback: drag-point maps are not represented by ScriptValue,
    // so only a runtime-specific hook can provide drag handles.
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNeutralCallback(_dragPoints, { lc::scripting::ScriptValue(self) });
    return result;
}

void ScriptCustomEntity::onDragPointClick(lc::operation::Builder_SPtr builder, unsigned int point) const {
    auto hook = lc::scripting::customEntityDispatchHook(_dragPointClick);
    if (hook && hook->tryOnDragPointClick(_dragPointClick, shared_from_this(),
                                          builder, point)) {
        return;
    }

    // Neutral fallback cannot represent Builder_SPtr; language adapters
    // must supply the typed path for transaction-aware callbacks.
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNeutralCallback(_dragPointClick, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue{},  // Builder_SPtr — no kind yet
        lc::scripting::ScriptValue(static_cast<int>(point)),
    });
}

void ScriptCustomEntity::onDragPointRelease(lc::operation::Builder_SPtr builder) const {
    auto hook = lc::scripting::customEntityDispatchHook(_dragPointRelease);
    if (hook && hook->tryOnDragPointRelease(_dragPointRelease, shared_from_this(),
                                            builder)) {
        return;
    }

    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNeutralCallback(_dragPointRelease, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue{},  // Builder_SPtr — no kind yet
    });
}

void ScriptCustomEntity::setDragPoint(lc::geo::Coordinate position) const {
    auto hook = lc::scripting::customEntityDispatchHook(_newDragPoint);
    if (hook && hook->trySetDragPoint(_newDragPoint, shared_from_this(),
                                      position)) {
        return;
    }

    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNeutralCallback(_newDragPoint, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue(position),
    });
}
