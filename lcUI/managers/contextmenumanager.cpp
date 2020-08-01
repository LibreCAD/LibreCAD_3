#include "contextmenumanager.h"
#include "widgets/guiAPI/menuitem.h"

using namespace lc::ui;

ContextMenuManager::ContextMenuManager(lua_State* luastate) 
    :
    _L(luastate)
{}

void ContextMenuManager::addOperation(const std::string& key, const std::string& groupName) {
    if (_operationMap.find(groupName) == _operationMap.end()) {
        _operationMap[groupName] = std::vector<std::string>();
    }

    _operationMap[groupName].push_back(key);
}

void ContextMenuManager::generateMenu(api::Menu* menu, std::vector<lc::entity::CADEntity_CSPtr> selectedEntities) {
    if (selectedEntities.size() > 0) {
        menu->addSection("Modify");

        for (const std::string& opName : _operationMap["Modify"]) {
            lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(opName.c_str());
            _L.dostring("contextmenu_op = function() run_basic_operation(" + opName + ") end");
            opItem->addCallback(_L["contextmenu_op"]);
            menu->addItem(opItem);
        }
    }
    else {
        menu->addSection("Creation");
        for (const std::string& opName : _operationMap["Creation"]) {
            lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(opName.c_str());
            _L.dostring("contextmenu_op = function() run_basic_operation(" + opName + ") end");
            opItem->addCallback(_L["contextmenu_op"]);
            menu->addItem(opItem);
        }

        menu->addSeparator();
        menu->addSection("Dimensions");
        for (const std::string& opName : _operationMap["Dimensions"]) {
            lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(opName.c_str());
            _L.dostring("contextmenu_op = function() run_basic_operation(" + opName + ") end");
            opItem->addCallback(_L["contextmenu_op"]);
            menu->addItem(opItem);
        }
    }
}
