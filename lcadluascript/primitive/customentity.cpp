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
// callback isn't Lua-backed (caller falls through to whatever
// non-Lua handling — but for the 6 slots here, non-Lua callbacks
// are unsupported until sub-piece 3 adds Python bindings for the
// custom-entity behavior surface).
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
    // Non-Lua path (Python/native): unsupported until sub-piece 3
    // adds Python bindings for the custom-entity behavior slots.
    // Returning empty is safe — the receiver treats empty snap as
    // "this entity doesn't contribute snap points".
    return points;
}

geo::Coordinate ScriptCustomEntity::nearestPointOnPath(const geo::Coordinate& coord) const {
    geo::Coordinate result;
    if (tryLuaCallReturning(_nearestPoint, result,
                            shared_from_this(), coord)) {
        return result;
    }
    // Non-Lua path unsupported — return the query point as a safe
    // fallback (matches the pre-refactor "no valid callback" case
    // which would have propagated an unspecified value from the nil
    // LuaRef's call<T>()).
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
    return result;
}

void ScriptCustomEntity::onDragPointClick(lc::operation::Builder_SPtr builder, unsigned int point) const {
    tryLuaCallVoid(_dragPointClick, shared_from_this(), builder, point);
    // Non-Lua path — no-op until sub-piece 3.
}

void ScriptCustomEntity::onDragPointRelease(lc::operation::Builder_SPtr builder) const {
    tryLuaCallVoid(_dragPointRelease, shared_from_this(), builder);
}

void ScriptCustomEntity::setDragPoint(lc::geo::Coordinate position) const {
    tryLuaCallVoid(_newDragPoint, shared_from_this(), position);
}
