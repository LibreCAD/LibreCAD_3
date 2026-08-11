#include "customentitydispatch_lua.h"

#include <lcscripting/builders/customentity.h>

#include "luacallback.h"  // unwrapLuaCallback + makeLuaCallback

#include <cad/primitive/insert.h>
// kaguya's `push()` uses typeid() on the pointee type; Builder must be
// a complete type at each `luaRef(builder, ...)` call site below.
#include <cad/operations/builder.h>

// Phase 6 PR-6.1 sub-piece 2b — Lua dispatch hook implementation.
//
// All 6 try* methods follow the same pattern established by sub-piece
// 1's fixup (`LuaCustomEntityManager::dispatchToPlugin`) and applied
// uniformly by sub-piece 2a's dispatch helpers:
//
//   1. Ask `unwrapLuaCallback` for the underlying `kaguya::LuaRef`.
//   2. If the ref isn't a Lua function, return false — caller falls
//      through to `callback.invoke(...)` per the neutral path.
//   3. Otherwise call the LuaRef directly with the concrete-typed args
//      (kaguya deduces `T = Insert` from the STATIC template arg on
//      `shared_ptr<Insert>`, so the Lua userdata gets Insert's
//      metatable — Lua-side `insert:document()` etc. resolve).

using namespace lc;
using namespace lc::lua;

bool LuaCustomEntityDispatchHook::trySnapPoints(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    const lc::geo::Coordinate& coord,
    const lc::SimpleSnapConstrain& simpleSnapConstrain,
    double minDistanceToSnap,
    int maxNumberOfSnapPoints,
    std::vector<lc::EntityCoordinate>& out) {
    kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(callback);
    if (luaRef.type() != LUA_TFUNCTION) {
        return false;
    }
    out = luaRef.call<std::vector<lc::EntityCoordinate>>(
        self, coord, simpleSnapConstrain, minDistanceToSnap, maxNumberOfSnapPoints);
    return true;
}

bool LuaCustomEntityDispatchHook::tryNearestPointOnPath(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    const lc::geo::Coordinate& coord,
    lc::geo::Coordinate& out) {
    kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(callback);
    if (luaRef.type() != LUA_TFUNCTION) {
        return false;
    }
    out = luaRef.call<lc::geo::Coordinate>(self, coord);
    return true;
}

bool LuaCustomEntityDispatchHook::tryDragPoints(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    std::map<unsigned int, lc::geo::Coordinate>& out) {
    kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(callback);
    if (luaRef.type() != LUA_TFUNCTION) {
        return false;
    }
    out = luaRef.call<std::map<unsigned int, lc::geo::Coordinate>>(self);
    return true;
}

bool LuaCustomEntityDispatchHook::trySetDragPoint(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    const lc::geo::Coordinate& position) {
    kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(callback);
    if (luaRef.type() != LUA_TFUNCTION) {
        return false;
    }
    luaRef(self, position);
    return true;
}

bool LuaCustomEntityDispatchHook::tryOnDragPointClick(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    lc::operation::Builder_SPtr builder,
    unsigned int point) {
    kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(callback);
    if (luaRef.type() != LUA_TFUNCTION) {
        return false;
    }
    luaRef(self, builder, point);
    return true;
}

bool LuaCustomEntityDispatchHook::tryOnDragPointRelease(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    lc::operation::Builder_SPtr builder) {
    kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(callback);
    if (luaRef.type() != LUA_TFUNCTION) {
        return false;
    }
    luaRef(self, builder);
    return true;
}

// -----------------------------------------------------------------------------
// Hook installer.
// -----------------------------------------------------------------------------

namespace lc {
namespace lua {

void installLuaCustomEntityDispatchHook() {
    lc::scripting::setCustomEntityDispatchHook(
        std::make_shared<LuaCustomEntityDispatchHook>());
}

// -----------------------------------------------------------------------------
// Free LuaRef→ScriptCallback wrapper helpers with LUA_TFUNCTION guard.
// -----------------------------------------------------------------------------

namespace {

// Convert a kaguya::LuaRef to a ScriptCallback ONLY IF it's a function.
// Non-function refs (nil, table, string, etc.) return a nil
// ScriptCallback so `checkValues()` correctly reports the slot as
// missing.  This preserves sub-piece 2a's fixup semantic verbatim.
lc::scripting::ScriptCallback wrapIfFunction(const kaguya::LuaRef& fn) {
    if (fn.type() != LUA_TFUNCTION) {
        return {};  // default-constructed nil ScriptCallback
    }
    return lc::lua::makeLuaCallback(fn);
}

} // namespace

void setSnapFunctionLua(lc::builder::CustomEntityBuilder& builder,
                        const kaguya::LuaRef& fn) {
    builder.setSnapFunction(wrapIfFunction(fn));
}

void setNearestPointFunctionLua(lc::builder::CustomEntityBuilder& builder,
                                const kaguya::LuaRef& fn) {
    builder.setNearestPointFunction(wrapIfFunction(fn));
}

void setDragPointsFunctionLua(lc::builder::CustomEntityBuilder& builder,
                              const kaguya::LuaRef& fn) {
    builder.setDragPointsFunction(wrapIfFunction(fn));
}

void setNewDragPointFunctionLua(lc::builder::CustomEntityBuilder& builder,
                                const kaguya::LuaRef& fn) {
    builder.setNewDragPointFunction(wrapIfFunction(fn));
}

void setDragPointsClickedFunctionLua(lc::builder::CustomEntityBuilder& builder,
                                     const kaguya::LuaRef& fn) {
    builder.setDragPointsClickedFunction(wrapIfFunction(fn));
}

void setDragPointsReleasedFunctionLua(lc::builder::CustomEntityBuilder& builder,
                                      const kaguya::LuaRef& fn) {
    builder.setDragPointsReleasedFunction(wrapIfFunction(fn));
}

} // namespace lua
} // namespace lc
