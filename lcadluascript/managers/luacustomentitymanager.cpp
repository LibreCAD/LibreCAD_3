#include <cad/events/newwaitingcustomentityevent.h>
#include <cad/storage/documentlist.h>
#include <cad/meta/customentitystorage.h>
#include <cad/primitive/insert.h>
#include "luacustomentitymanager.h"

// Phase 6 PR-6.1 — Lua adapter needed to wrap the LuaRef overload's
// callback into a ScriptCallback before storing, AND to unwrap the
// ScriptCallback on dispatch so the concrete Insert type reaches the
// Lua callback with the correct kaguya metatable.
#include <scriptadapter/luacallback.h>

using namespace lc::lua;

namespace {

// Phase 6 PR-6.1 sub-piece 1 fixup — concrete-type-preserving dispatch.
//
// The bug the coordinator's PR-6.1 sub-piece 1 review caught:
// packaging `Insert_CSPtr` as `ScriptValue::Entity` upcasts to
// `CADEntity_CSPtr` at compile time.  When the Lua adapter's
// `toLua()` unpacks that ScriptValue and hands the pointer to kaguya's
// `LuaRef` constructor, kaguya's `lua_type_traits<shared_ptr<T>>::push()`
// deduces `T = CADEntity` from the STATIC template argument (verified
// against `third_party/kaguya/include/kaguya/type.hpp`).  The pushed Lua
// userdata gets `CADEntity`'s metatable — which lacks Insert-specific
// methods like `document()` / `displayBlock()` / `position()`.  Every
// existing Lua custom-entity plugin (rectangle, fractal, spiral, gear,
// star) reaches for `insert:document()` and would fail post-refactor.
//
// The fix: `unwrapLuaCallback()` returns the underlying `kaguya::LuaRef`
// when the callback was produced by `makeLuaCallback` (runtime tag ==
// "lua").  Calling the LuaRef DIRECTLY with the concrete `Insert_CSPtr`
// lets kaguya deduce `T = Insert` correctly; the pushed userdata gets
// Insert's metatable and Lua-side `insert:document()` works.
//
// Non-Lua callbacks (native / python) go through the neutral
// ScriptValue path.  For Python, `py::cast(shared_ptr<T>)` uses
// pybind11's RTTI-based downcast (via `py::classh` polymorphic
// dispatch) — the concrete Insert type is recovered at cast time.
// This is verified by the manager-dispatch test that fires a real
// event through a native callback and asserts on the recovered
// concrete-type pointer.
void dispatchToPlugin(const lc::scripting::ScriptCallback& callback,
                      const lc::entity::Insert_CSPtr& insert) {
    // Lua fast path: bypass the ScriptValue round-trip entirely so
    // kaguya sees the concrete Insert_CSPtr at push time.
    kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(callback);
    if (luaRef.type() == LUA_TFUNCTION) {
        // Directly call the LuaRef with the concrete Insert_CSPtr.
        // kaguya deduces T=Insert from the argument type, pushes with
        // Insert's metatable, so Lua-side `insert:document()` etc.
        // resolve correctly.  Preserves the pre-refactor semantics
        // verbatim.
        luaRef(insert);
        return;
    }

    // Non-Lua path: neutral ScriptValue dispatch.  Python's adapter
    // uses `py::cast(CADEntity_CSPtr)` which does RTTI-based downcast
    // via pybind11's polymorphic classh dispatch — the concrete Insert
    // type is recovered on the Python side.  Native callbacks receive
    // the ScriptValue and can `dynamic_pointer_cast<Insert>` on the
    // stored CADEntity_CSPtr.
    lc::entity::CADEntity_CSPtr insertAsEntity = insert;
    callback.invoke({lc::scripting::ScriptValue(insertAsEntity)});
}

} // namespace

LuaCustomEntityManager::LuaCustomEntityManager() {
    storage::DocumentList::getInstance().newWaitingCustomEntityEvent().connect<LuaCustomEntityManager, &LuaCustomEntityManager::onNewWaitingEntity>(this);
}

LuaCustomEntityManager::~LuaCustomEntityManager() {
    _plugins.clear();
    storage::DocumentList::getInstance().newWaitingCustomEntityEvent().disconnect<LuaCustomEntityManager, &LuaCustomEntityManager::onNewWaitingEntity>(this);
}


void LuaCustomEntityManager::onNewWaitingEntity(const lc::event::NewWaitingCustomEntityEvent& event) {
    auto block = event.insert()->displayBlock();

    auto ces = std::static_pointer_cast<const lc::meta::CustomEntityStorage>(block);
    if(!ces) {
        return;
    }

    auto it = _plugins.find(ces->pluginName());
    if(it == _plugins.end()) {
        return;
    }

    // Phase 6 PR-6.1 sub-piece 1 fixup — dispatch preserves the
    // concrete Insert type.  See dispatchToPlugin() rationale above.
    dispatchToPlugin(it->second, event.insert());
}

void LuaCustomEntityManager::registerPlugin(const std::string& name, kaguya::LuaRef onNewWaitingEntityFunction) {
    if(onNewWaitingEntityFunction.type() != LUA_TFUNCTION) {
        return;
    }

    // Phase 6 PR-6.1 — wrap the LuaRef into a neutral ScriptCallback
    // and forward to the ScriptCallback overload.  All the
    // "fire on already-waiting entities" replay logic lives there.
    registerPlugin(name,
        lc::lua::makeLuaCallback(std::move(onNewWaitingEntityFunction)));
}

void LuaCustomEntityManager::registerPlugin(const std::string& name,
                                            lc::scripting::ScriptCallback callback) {
    if (callback.isNil()) {
        return;
    }

    _plugins[name] = callback;

    // Replay: fire the just-registered plugin for any waiting entities
    // in the process-global DocumentList.  Same behavior as the
    // pre-refactor LuaRef path.  Uses the SAME concrete-type-preserving
    // dispatch helper as onNewWaitingEntity — otherwise Lua plugins
    // registered AFTER a DXF was opened would receive base-typed
    // userdata for replayed entities (bug symmetry with the primary
    // dispatch site).
    for (const auto& insert : storage::DocumentList::getInstance().waitingCustomEntities(name)) {
        dispatchToPlugin(callback, insert);
    }
}

void LuaCustomEntityManager::removePlugins() {
    _plugins.clear();
}

bool LuaCustomEntityManager::hasPlugin(const std::string& name) const {
    return _plugins.find(name) != _plugins.end();
}
