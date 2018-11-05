#include <cad/events/newwaitingcustomentityevent.h>
#include <cad/storage/documentlist.h>
#include <cad/meta/customentitystorage.h>
#include <cad/primitive/insert.h>
#include "luacustomentitymanager.h"

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

    it->second(event.insert());
}

void LuaCustomEntityManager::registerPlugin(const std::string& name, kaguya::LuaRef onNewWaitingEntityFunction) {
    if(onNewWaitingEntityFunction.type() != LUA_TFUNCTION) {
        return;
    }

    _plugins[name] = onNewWaitingEntityFunction;

    for(auto entity : storage::DocumentList::getInstance().waitingCustomEntities(name)) {
        onNewWaitingEntityFunction(entity);
    }
}

void LuaCustomEntityManager::removePlugins() {
    _plugins.clear();
}