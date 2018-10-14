#include <cad/events/newwaitingcustomentityevent.h>
#include <cad/dochelpers/documentlist.h>
#include <cad/meta/customentitystorage.h>
#include <cad/primitive/insert.h>
#include "luacustomentitymanager.h"

using namespace lc::lua;

LuaCustomEntityManager::LuaCustomEntityManager() {
    DocumentList::getInstance().newWaitingCustomEntityEvent().connect<LuaCustomEntityManager, &LuaCustomEntityManager::onNewWaitingEntity>(this);
}

LuaCustomEntityManager::~LuaCustomEntityManager() {
    _plugins.clear();
    DocumentList::getInstance().newWaitingCustomEntityEvent().disconnect<LuaCustomEntityManager, &LuaCustomEntityManager::onNewWaitingEntity>(this);
}


void LuaCustomEntityManager::onNewWaitingEntity(const lc::NewWaitingCustomEntityEvent& event) {
    auto block = event.insert()->displayBlock();

    auto ces = std::static_pointer_cast<const lc::CustomEntityStorage>(block);
    if(!ces) {
        return;
    }

    auto it = _plugins.find(ces->pluginName());
    if(it == _plugins.end()) {
        return;
    }

    it->second(event.insert());
}

void LuaCustomEntityManager::registerPlugin(const std::string& name, LuaIntf::LuaRef onNewWaitingEntityFunction) {
    if(!onNewWaitingEntityFunction.isValid() || !onNewWaitingEntityFunction.isFunction()) {
        return;
    }

    _plugins[name] = onNewWaitingEntityFunction;

    for(auto entity : DocumentList::getInstance().waitingCustomEntities(name)) {
        onNewWaitingEntityFunction(entity);
    }
}

void LuaCustomEntityManager::removePlugins() {
    _plugins.clear();
}