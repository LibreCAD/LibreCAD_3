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
    if (_mainWindow->cadMdiChild()->viewer()->operationActive()) {
        activeCommands(menu, selectedEntities);
    }else if (selectedEntities.size() > 0) {
        selectedCommands(menu, selectedEntities);
    }
    else {
        inactiveCommands(menu);
    }
}

void ContextMenuManager::activeCommands(api::Menu* menu, const std::vector<lc::entity::CADEntity_CSPtr>& selectedEntities) {
    // Cancel command
    api::MenuItem* cancelItem = new api::MenuItem("Cancel");
    _L.dostring("contextmenu_op = function() finish_operation() end");
    cancelItem->addCallback(_L["contextmenu_op"]);
    menu->addItem(cancelItem);

    // Snap commands
    addSnapCommands(menu);
}

void ContextMenuManager::inactiveCommands(api::Menu* menu) {
    api::MenuItem* lastCommandItem = new api::MenuItem("Last Command");
    api::MenuItem* undoItem = new api::MenuItem("Undo");
    api::MenuItem* redoItem = new api::MenuItem("Redo");

    _L.dostring("contextmenu_op = function() mainWindow:runLastOperation() end");
    lastCommandItem->addCallback(_L["contextmenu_op"]);

    _L.dostring("contextmenu_op = function() mainWindow:undo() mainWindow:undo() end");
    undoItem->addCallback(_L["contextmenu_op"]);

    _L.dostring("contextmenu_op = function() mainWindow:redo() mainWindow:redo() end");
    redoItem->addCallback(_L["contextmenu_op"]);

    menu->addItem(lastCommandItem);
    menu->addItem(undoItem);
    menu->addItem(redoItem);

    // Select commands
    api::Menu* selectMenu = new api::Menu("Select");
    api::MenuItem* selectAllItem = new api::MenuItem("Select All");
    _L.dostring("contextmenu_op = function() mainWindow:selectAll() end");
    selectAllItem->addCallback(_L["contextmenu_op"]);

    selectMenu->addItem(selectAllItem);
    menu->addMenu(selectMenu);

    // Creation commands
    api::Menu* groupMenu = new api::Menu("Creation");
    for (const std::string& opName : _operationMap["Creation"]) {
        lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(cleanOperationName(opName).c_str());
        _L.dostring("contextmenu_op = function() run_basic_operation(" + opName + ") end");
        opItem->addCallback(_L["contextmenu_op"]);
        groupMenu->addItem(opItem);
    }

    menu->addMenu(groupMenu);

    // Dimension commands
    api::Menu* groupMenu2 = new api::Menu("Dimensions");
    for (const std::string& opName : _operationMap["Dimensions"]) {
        lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(cleanOperationName(opName).c_str());
        _L.dostring("contextmenu_op = function() run_basic_operation(" + opName + ") end");
        opItem->addCallback(_L["contextmenu_op"]);
        groupMenu2->addItem(opItem);
    }

    menu->addMenu(groupMenu2);
}

void ContextMenuManager::selectedCommands(api::Menu* menu, const std::vector<lc::entity::CADEntity_CSPtr>& selectedEntities) {
    // Modify commands
    api::Menu* groupMenu = new api::Menu("Modify");
    for (const std::string& opName : _operationMap["Modify"]) {
        lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(cleanOperationName(opName).c_str());
        _L.dostring("contextmenu_op = function() run_basic_operation(" + opName + ") end");
        opItem->addCallback(_L["contextmenu_op"]);
        groupMenu->addItem(opItem);
    }
    menu->addMenu(groupMenu);

    // Select commands
    api::Menu* selectMenu = new api::Menu("Select");
    api::MenuItem* selectAllItem = new api::MenuItem("Select All");
    api::MenuItem* selectNoneItem = new api::MenuItem("Select None");
    api::MenuItem* selectInverted = new api::MenuItem("Invert Selection");

    _L.dostring("contextmenu_op = function() mainWindow:selectAll() end");
    selectAllItem->addCallback(_L["contextmenu_op"]);

    _L.dostring("contextmenu_op = function() mainWindow:selectNone() end");
    selectNoneItem->addCallback(_L["contextmenu_op"]);

    _L.dostring("contextmenu_op = function() mainWindow:invertSelection() end");
    selectInverted->addCallback(_L["contextmenu_op"]);

    selectMenu->addItem(selectAllItem);
    selectMenu->addItem(selectNoneItem);
    selectMenu->addItem(selectInverted);
    menu->addMenu(selectMenu);
}

std::string ContextMenuManager::cleanOperationName(const std::string& opName) const {
    std::string result = opName;
    result = result.substr(0, result.find("Operation"));
    return result;
}

void ContextMenuManager::addSnapCommands(api::Menu* menu) {
    api::Menu* snapMenu = new api::Menu("Snap");
    api::MenuItem* snapGridItem = new api::MenuItem("Grid");
    api::MenuItem* snapIntersectionItem = new api::MenuItem("Intersection");
    api::MenuItem* snapMiddleItem = new api::MenuItem("Middle");
    api::MenuItem* snapEntityItem = new api::MenuItem("Entity");

    snapGridItem->setCheckable(true);
    snapIntersectionItem->setCheckable(true);
    snapMiddleItem->setCheckable(true);
    snapEntityItem->setCheckable(true);

    // check current state of snaps
    const viewer::manager::SnapManagerImpl_SPtr snapManager = _mainWindow->cadMdiChild()->getSnapManager();

    snapGridItem->setChecked(snapManager->isGridSnappable());
    snapIntersectionItem->setChecked(snapManager->isIntersectionsSnappable());
    lc::SimpleSnapConstrain snapConstrain = snapManager->snapConstrain();
    snapMiddleItem->setChecked(snapConstrain.hasConstrain(lc::SimpleSnapConstrain::LOGICAL));
    snapEntityItem->setChecked(snapConstrain.hasConstrain(lc::SimpleSnapConstrain::ON_ENTITY));

    // add callbacks
    _L.dostring("contextmenu_op = function(enabled) mainWindow:cadMdiChild():getSnapManager():setGridSnappable(enabled) mainWindow:toolbar():updateSnapButtons(mainWindow:cadMdiChild():getSnapManager()) end");
    snapGridItem->addCheckedCallback(_L["contextmenu_op"]);

    _L.dostring("contextmenu_op = function(enabled) mainWindow:cadMdiChild():getSnapManager():setIntersectionSnappable(enabled) mainWindow:toolbar():updateSnapButtons(mainWindow:cadMdiChild():getSnapManager()) end");
    snapIntersectionItem->addCheckedCallback(_L["contextmenu_op"]);

    _L.dostring("contextmenu_op = function(enabled) mainWindow:cadMdiChild():getSnapManager():setMiddleSnappable(enabled) mainWindow:toolbar():updateSnapButtons(mainWindow:cadMdiChild():getSnapManager()) end");
    snapMiddleItem->addCheckedCallback(_L["contextmenu_op"]);

    _L.dostring("contextmenu_op = function(enabled) mainWindow:cadMdiChild():getSnapManager():setEntitySnappable(enabled) mainWindow:toolbar():updateSnapButtons(mainWindow:cadMdiChild():getSnapManager()) end");
    snapEntityItem->addCheckedCallback(_L["contextmenu_op"]);

    snapMenu->addItem(snapGridItem);
    snapMenu->addItem(snapIntersectionItem);
    snapMenu->addItem(snapMiddleItem);
    snapMenu->addItem(snapEntityItem);
    menu->addMenu(snapMenu);
}
