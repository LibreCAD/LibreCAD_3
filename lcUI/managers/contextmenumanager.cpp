#include "contextmenumanager.h"
#include "widgets/guiAPI/menuitem.h"

using namespace lc::ui;

std::map<MainWindow*, int> ContextMenuManager::instancesId;
std::map<int, ContextMenuManager*> ContextMenuManager::instances;
int ContextMenuManager::_instanceCount = 0;

ContextMenuManager::ContextMenuManager(lc::ui::MainWindow* mainWindowIn)
    :
    _L(mainWindowIn->luaInterface()->luaState()),
    _mainWindow(mainWindowIn)
{}

ContextMenuManager* ContextMenuManager::GetContextMenuManager(lc::ui::MainWindow* mainWindowIn) {
    if (instancesId.find(mainWindowIn) == instancesId.end()) {
        instancesId[mainWindowIn] = _instanceCount;
        instances[_instanceCount] = new ContextMenuManager(mainWindowIn);
        _instanceCount++;
    }

    return instances[instancesId[mainWindowIn]];
}

ContextMenuManager* ContextMenuManager::GetContextMenuManager(int instanceIdIn) {
    if (instances.find(instanceIdIn) == instances.end()) {
        return nullptr;
    }

    return instances[instanceIdIn];
}

int ContextMenuManager::GetInstanceId(lc::ui::MainWindow* mainWindowIn) {
    if (instancesId.find(mainWindowIn) == instancesId.end()) {
        return -1;
    }

    return instancesId[mainWindowIn];
}

void ContextMenuManager::addOperation(const std::string& key, const std::string& groupName) {
    if (key == "CreateOperations" || key == "Operations") {
        return;
    }

    if (_operationMap.find(groupName) == _operationMap.end()) {
        _operationMap[groupName] = std::vector<std::string>();
    }

    _operationMap[groupName].push_back(key);
}

void ContextMenuManager::generateMenu(api::Menu* menu, std::vector<lc::entity::CADEntity_CSPtr> selectedEntities) {
    if (selectedEntities.size() > 0) {
        api::Menu* groupMenu = new api::Menu("Modify");

        for (const std::string& opName : _operationMap["Modify"]) {
            lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(cleanOperationName(opName).c_str());
            _L.dostring("contextmenu_op = function() run_basic_operation(" + opName + ") end");
            opItem->addCallback(_L["contextmenu_op"]);
            groupMenu->addItem(opItem);
        }

        menu->addMenu(groupMenu);
    }
    else {
        api::Menu* groupMenu = new api::Menu("Creation");
        for (const std::string& opName : _operationMap["Creation"]) {
            lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(cleanOperationName(opName).c_str());
            _L.dostring("contextmenu_op = function() run_basic_operation(" + opName + ") end");
            opItem->addCallback(_L["contextmenu_op"]);
            groupMenu->addItem(opItem);
        }

        menu->addMenu(groupMenu);

        api::Menu* groupMenu2 = new api::Menu("Dimensions");
        for (const std::string& opName : _operationMap["Dimensions"]) {
            lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(cleanOperationName(opName).c_str());
            _L.dostring("contextmenu_op = function() run_basic_operation(" + opName + ") end");
            opItem->addCallback(_L["contextmenu_op"]);
            groupMenu2->addItem(opItem);
        }

        menu->addMenu(groupMenu2);
    }
}

std::string ContextMenuManager::cleanOperationName(const std::string& opName) const {
    std::string result = opName;
    result = result.substr(0, result.find("Operation"));
    return result;
}
