#include <cad/events/newwaitingcustomentityevent.h>
#include <cad/storage/documentlist.h>
#include <cad/meta/customentitystorage.h>
#include <cad/primitive/insert.h>
#include "luacustomentitymanager.h"

// Phase 6 PR-6.1 — Lua adapter needed to wrap the LuaRef overload's
// callback into a ScriptCallback before storing.
#include <scriptadapter/luacallback.h>

using namespace lc::lua;

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

    // Phase 6 PR-6.1 — dispatch through the neutral ScriptCallback.
    // The insert (Insert_CSPtr = shared_ptr<const Insert>) upcasts to
    // CADEntity_CSPtr implicitly via public inheritance; wrap into a
    // ScriptValue::Entity so the adapter (Lua or Python) materializes
    // the concrete type on the receiver side (Lua's polymorphic
    // userdata + kaguya downcast, or pybind11's classh downcast).
    //
    // Before this refactor: `it->second(event.insert())` invoked kaguya
    // directly and worked because kaguya recognized Insert_CSPtr as a
    // bound type.  The neutral path preserves the same behavior via
    // ScriptValue -> py::cast / kaguya::LuaRef materialization.
    lc::entity::CADEntity_CSPtr insertAsEntity = event.insert();
    it->second.invoke({lc::scripting::ScriptValue(insertAsEntity)});
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
    // pre-refactor LuaRef path.  Wrap each insert in a ScriptValue for
    // the neutral dispatch.
    for (auto entity : storage::DocumentList::getInstance().waitingCustomEntities(name)) {
        lc::entity::CADEntity_CSPtr insertAsEntity = entity;
        callback.invoke({lc::scripting::ScriptValue(insertAsEntity)});
    }
}

void LuaCustomEntityManager::removePlugins() {
    _plugins.clear();
}

bool LuaCustomEntityManager::hasPlugin(const std::string& name) const {
    return _plugins.find(name) != _plugins.end();
}
