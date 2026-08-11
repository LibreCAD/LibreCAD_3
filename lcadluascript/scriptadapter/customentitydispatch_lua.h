#pragma once

// Phase 6 PR-6.1 sub-piece 2b — Lua adapter for the CustomEntity
// concrete-type-preserving dispatch hook.  Implements
// `lc::scripting::CustomEntityDispatchHook` using `unwrapLuaCallback` +
// `luaRef.call<T>()` — the fast path that lets kaguya's static
// template deduction push `shared_ptr<Insert>` with Insert's metatable
// (so Lua-side `insert:document()` / `insert:displayBlock()` /
// `insert:position()` resolve correctly).
//
// Also provides free wrapper functions for the LuaRef→ScriptCallback
// path with the LUA_TFUNCTION guard from sub-piece 2a's fixup preserved
// (non-callable LuaRefs silently no-op, matching the pre-refactor
// "reject at set time" semantic).

#include <lcscripting/customentitydispatchhook.h>
#include <lcscripting/scriptcallback.h>

#include <kaguya/kaguya.hpp>

namespace lc {
namespace builder {
class CustomEntityBuilder;
}
}

namespace lc {
namespace lua {

/// Concrete `CustomEntityDispatchHook` for Lua callbacks.  Falls back
/// (returns false) if the ScriptCallback wasn't produced by
/// `makeLuaCallback` — the ScriptCustomEntity dispatch then routes to
/// `callback.invoke(...)` per the neutral path.
class LuaCustomEntityDispatchHook : public lc::scripting::CustomEntityDispatchHook {
public:
    bool trySnapPoints(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        const lc::geo::Coordinate& coord,
        const lc::SimpleSnapConstrain& simpleSnapConstrain,
        double minDistanceToSnap,
        int maxNumberOfSnapPoints,
        std::vector<lc::EntityCoordinate>& out) override;

    bool tryNearestPointOnPath(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        const lc::geo::Coordinate& coord,
        lc::geo::Coordinate& out) override;

    bool tryDragPoints(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        std::map<unsigned int, lc::geo::Coordinate>& out) override;

    bool trySetDragPoint(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        const lc::geo::Coordinate& position) override;

    bool tryOnDragPointClick(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        lc::operation::Builder_SPtr builder,
        unsigned int point) override;

    bool tryOnDragPointRelease(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        lc::operation::Builder_SPtr builder) override;
};

/// Install the Lua dispatch hook as the process-global
/// `CustomEntityDispatchHook`.  Idempotent — safe to call multiple
/// times (later calls overwrite with a fresh instance; the previous
/// instance's ref-count drops to zero and it's destroyed).
///
/// Called from `LCLua::LCLua(...)` in `lcadluascript/lclua.cpp` so
/// every window's initLua step installs the hook (matches Lua-adapter
/// initialization ordering).
void installLuaCustomEntityDispatchHook();

// -----------------------------------------------------------------------------
// Free wrappers: LuaRef → ScriptCallback with LUA_TFUNCTION guard.
// -----------------------------------------------------------------------------
//
// Reference for sub-piece 2a's fixup: the LUA_TFUNCTION guard rejects
// non-callable LuaRefs at set time so the slot stays default-constructed
// nil and `checkValues()` correctly reports it as missing.  Post-move,
// `CustomEntityBuilder` in lcscripting takes ScriptCallback only — these
// helpers provide the Lua-adapter-side glue that preserves the same
// semantic.
//
// Usage:
//   kaguya::LuaRef fn = state["my_snap"];
//   lc::lua::setSnapFunctionLua(builder, fn);
//
// A non-function `fn` is silently rejected (leaves the slot nil).

void setSnapFunctionLua(lc::builder::CustomEntityBuilder& builder,
                        const kaguya::LuaRef& fn);
void setNearestPointFunctionLua(lc::builder::CustomEntityBuilder& builder,
                                const kaguya::LuaRef& fn);
void setDragPointsFunctionLua(lc::builder::CustomEntityBuilder& builder,
                              const kaguya::LuaRef& fn);
void setNewDragPointFunctionLua(lc::builder::CustomEntityBuilder& builder,
                                const kaguya::LuaRef& fn);
void setDragPointsClickedFunctionLua(lc::builder::CustomEntityBuilder& builder,
                                     const kaguya::LuaRef& fn);
void setDragPointsReleasedFunctionLua(lc::builder::CustomEntityBuilder& builder,
                                      const kaguya::LuaRef& fn);

} // namespace lua
} // namespace lc
