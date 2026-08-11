#include <builders/customentity.h>
#include "customentity.h"

// Phase 6 PR-6.1 sub-piece 2a — Lua adapter for the concrete-type
// fast-path dispatch.  Same discipline as the sub-piece 1 fixup for
// LuaCustomEntityManager: `unwrapLuaCallback` on the ScriptCallback
// returns the underlying `kaguya::LuaRef`; calling it DIRECTLY with the
// concrete `shared_from_this()` result lets kaguya's
// `lua_type_traits<shared_ptr<T>>::push()` deduce `T = Insert` from
// the STATIC template argument, preserving Insert's metatable so
// Lua-side `insert:document()` / `insert:displayBlock()` /
// `insert:position()` calls resolve correctly.
#include <scriptadapter/luacallback.h>

using namespace lc;
using namespace entity;

namespace {

// Phase 6 PR-6.1 sub-piece 2a — Lua fast-path helper for dispatches
// that need to RETURN a typed value (snapPoints, nearestPointOnPath,
// dragPoints).  Runs the callback via kaguya's `luaRef.call<T>()`
// which handles the return-type materialization from Lua's return
// values.  Returns `true` and sets `out` on success; `false` if the
// callback isn't Lua-backed — the caller then falls through to the
// neutral ScriptValue path (see the six dispatch methods below).
template <typename Ret, typename... Args>
bool tryLuaCallReturning(const lc::scripting::ScriptCallback& callback,
                         Ret& out, Args&&... args) {
    kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(callback);
    if (luaRef.type() != LUA_TFUNCTION) {
        return false;
    }
    out = luaRef.call<Ret>(std::forward<Args>(args)...);
    return true;
}

// Lua fast-path helper for dispatches that DON'T return a value
// (setDragPoint, onDragPointClick, onDragPointRelease).  Returns
// `true` if the Lua path fired.
template <typename... Args>
bool tryLuaCallVoid(const lc::scripting::ScriptCallback& callback,
                    Args&&... args) {
    kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(callback);
    if (luaRef.type() != LUA_TFUNCTION) {
        return false;
    }
    luaRef(std::forward<Args>(args)...);
    return true;
}

// Phase 6 PR-6.1 sub-piece 2a review fixup — non-Lua fallback:
// invoke the callback through the neutral ScriptValue path so
// native (or future Python) callbacks actually FIRE.
//
// Motivation (coordinator's review): pre-fixup, if the callback
// wasn't Lua-backed, dispatch silently returned a hardcoded default
// (empty vector / query point / empty map / no-op) — never invoking
// the ScriptCallback.  A native caller passing a real callable
// through `nativeCallback` would satisfy `checkValues()`, build
// successfully, and have their callback silently never fire.  This
// fix routes non-Lua callbacks through the neutral `.invoke()` path
// so side effects reach the callee.
//
// Argument packing: `shared_from_this()` upcasts to CADEntity_CSPtr
// (via Insert's public inheritance).  Other args map to their
// ScriptValue kinds where available; unpackable types (Builder_SPtr,
// SimpleSnapConstrain) go through as Nil for now.  The neutral
// ScriptValue surface will grow those kinds in later phases when
// Python custom-entity plugins need them.
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
    // Fast path: direct kaguya call preserves shared_from_this()'s
    // concrete Insert type so Lua-side `insert:document()` etc. work.
    // shared_from_this() returns shared_ptr<Insert> here (Insert
    // derives from enable_shared_from_this<Insert>), which is
    // sufficient — the Lua plugins call Insert-level methods.
    if (tryLuaCallReturning(_snapPoints, points,
                            shared_from_this(), coord, simpleSnapConstrain,
                            minDistanceToSnap, maxNumberOfSnapPoints)) {
        Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
        return points;
    }
    // Non-Lua path: native / (future) Python callback.  Fire through
    // the neutral ScriptValue pipeline so side effects reach the
    // callee.  The return value (`vector<EntityCoordinate>`) has no
    // corresponding ScriptValue kind today, so the callback's return
    // is IGNORED — this dispatch site is for side effects only until
    // ScriptValue grows an EntityCoordinate list kind.  Documented at
    // the neutral-callback registration site so callers know.
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNonLua(_snapPoints, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue(coord),
        // SimpleSnapConstrain has no ScriptValue kind yet — Nil placeholder.
        lc::scripting::ScriptValue{},
        lc::scripting::ScriptValue(minDistanceToSnap),
        lc::scripting::ScriptValue(maxNumberOfSnapPoints),
    });
    return points;
}

geo::Coordinate ScriptCustomEntity::nearestPointOnPath(const geo::Coordinate& coord) const {
    geo::Coordinate result;
    if (tryLuaCallReturning(_nearestPoint, result,
                            shared_from_this(), coord)) {
        return result;
    }
    // Non-Lua path: fire callback + try to extract a Coordinate return.
    // If the callback's return is a Coordinate kind, use it; otherwise
    // fall back to the query point (matches the old "no valid
    // callback" case).
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
    if (tryLuaCallReturning(_dragPoints, result, shared_from_this())) {
        return result;
    }
    // Non-Lua path: fire callback for side effects.  Return type
    // (`map<unsigned int, Coordinate>`) has no ScriptValue kind — same
    // limitation as snapPoints's vector<EntityCoordinate>.  Returned
    // ScriptValue is discarded; result stays empty.
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNonLua(_dragPoints, {
        lc::scripting::ScriptValue(self),
    });
    return result;
}

void ScriptCustomEntity::onDragPointClick(lc::operation::Builder_SPtr builder, unsigned int point) const {
    if (tryLuaCallVoid(_dragPointClick, shared_from_this(), builder, point)) return;
    // Non-Lua path: fire callback.  Builder_SPtr has no ScriptValue
    // kind today — passes as Nil.  Callers that need builder access
    // must use the Lua path (or wait for Python bindings to add a
    // dedicated kind in sub-piece 3).
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNonLua(_dragPointClick, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue{},  // Builder_SPtr — no kind yet
        lc::scripting::ScriptValue(static_cast<int>(point)),
    });
}

void ScriptCustomEntity::onDragPointRelease(lc::operation::Builder_SPtr builder) const {
    if (tryLuaCallVoid(_dragPointRelease, shared_from_this(), builder)) return;
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNonLua(_dragPointRelease, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue{},  // Builder_SPtr — no kind yet
    });
}

void ScriptCustomEntity::setDragPoint(lc::geo::Coordinate position) const {
    if (tryLuaCallVoid(_newDragPoint, shared_from_this(), position)) return;
    lc::entity::CADEntity_CSPtr self = shared_from_this();
    invokeNonLua(_newDragPoint, {
        lc::scripting::ScriptValue(self),
        lc::scripting::ScriptValue(position),
    });
}
