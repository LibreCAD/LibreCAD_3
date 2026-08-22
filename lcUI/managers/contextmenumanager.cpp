#include "contextmenumanager.h"

// Phase 4 PR-8 — every context-menu callback is a native ScriptCallback
// lambda.  All 19 `contextmenu_op` dostrings gone; the `ispaused` and
// `pauseCreation` readbacks now go through ScriptObject::callMethod on
// the CURRENT operation; the operation's `contextMenuOptions(menu)` fires
// via callMethod with an OpaquePtr(menu) payload the Lua adapter
// materializes into userdata (encoder registered in guibridge.cpp).
#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptobject.h>
#include <lcscripting/scriptvalue.h>
#include "widgets/guiAPI/menuitem.h"
#include "lua/opaquetags.h"

using namespace lc::ui;

std::map<MainWindow*, int> ContextMenuManager::instancesId;
std::map<int, ContextMenuManager*> ContextMenuManager::instances;
int ContextMenuManager::_instanceCount = 0;

ContextMenuManager::ContextMenuManager(lc::ui::MainWindow* mainWindowIn)
    :
    _mainWindow(mainWindowIn)
{
    // Phase 4 PR-8: kaguya::State _L member gone.  All context-menu
    // callbacks are native ScriptCallback lambdas that only need
    // `_mainWindow`.  This retires the pre-initLua constructed-first
    // ordering hazard the previous code had.
}

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
    // Phase 4 PR-8 — 3 `contextmenu_op` dostrings killed (Cancel + PAN
    // isPaused readback + PAN pauseCreation toggle).
    MainWindow* self = _mainWindow;

    // Cancel command — was `contextmenu_op = function() finish_operation() end`
    api::MenuItem* cancelItem = new api::MenuItem("Cancel");
    cancelItem->addCallback(lc::scripting::nativeCallback([self]() {
        self->luaInterface()->finishOperation();
    }));
    menu->addItem(cancelItem);

    operationContextCommands(menu, selectedEntities);

    // PAN command — was two dostrings: an `ispaused =
    // mainWindow:currentOperation():isPaused()` READ (LuaRef materialized
    // then `.get<bool>()`), plus a `pauseCreation(enablePan)` callback.
    // Both go through ScriptObject::callMethod on the CURRENT operation.
    api::MenuItem* panItem = new api::MenuItem("PAN");
    panItem->setCheckable(true);
    {
        lc::scripting::ScriptObject curOp = self->currentOperation();
        if (!curOp.isNil()) {
            panItem->setChecked(curOp.callMethod("isPaused").asBool());
        }
    }
    panItem->addCheckedCallback(
        lc::scripting::nativeCallback(
            [self](const std::vector<lc::scripting::ScriptValue>& args) {
                lc::scripting::ScriptObject curOp = self->currentOperation();
                if (!curOp.isNil()) {
                    curOp.callMethod("pauseCreation", args);
                }
                return lc::scripting::ScriptValue{};
            }));
    menu->addItem(panItem);

    // Snap commands
    addSnapCommands(menu);
}

void ContextMenuManager::inactiveCommands(api::Menu* menu) {
    // Phase 4 PR-8 — 8 dostrings killed: Last Command / Undo / Redo /
    // Paste / Select All, plus the Creation and Dimensions
    // `run_basic_operation` loops (both route through the native
    // `runOperationByName` added in PR-7).
    MainWindow* self = _mainWindow;

    api::MenuItem* lastCommandItem = new api::MenuItem("Last Command");
    api::MenuItem* undoItem = new api::MenuItem("Undo");
    api::MenuItem* redoItem = new api::MenuItem("Redo");

    lastCommandItem->addCallback(
        lc::scripting::nativeCallback([self]() { self->runLastOperation(); }));
    undoItem->addCallback(
        lc::scripting::nativeCallback([self]() { self->undo(); }));
    redoItem->addCallback(
        lc::scripting::nativeCallback([self]() { self->redo(); }));

    menu->addItem(lastCommandItem);
    menu->addItem(undoItem);
    menu->addItem(redoItem);

    // paste command
    api::MenuItem* pasteItem = new api::MenuItem("Paste");
    pasteItem->addCallback(
        lc::scripting::nativeCallback([self]() { self->pasteEvent(); }));
    menu->addItem(pasteItem);

    // Select commands
    api::Menu* selectMenu = new api::Menu("Select");
    api::MenuItem* selectAllItem = new api::MenuItem("Select All");
    selectAllItem->addCallback(
        lc::scripting::nativeCallback([self]() { self->selectAll(); }));

    selectMenu->addItem(selectAllItem);
    menu->addMenu(selectMenu);

    // Creation commands — each button routes through the PR-7 native
    // `runOperationByName(opName)` entry point.  `opName` is captured
    // BY VALUE per iteration so every menu item's lambda holds its own
    // operation-class name.
    api::Menu* groupMenu = new api::Menu("Creation");
    for (const std::string& opName : _operationMap["Creation"]) {
        lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(cleanOperationName(opName).c_str());
        opItem->addCallback(
            lc::scripting::nativeCallback([self, opName]() {
                self->runOperationByName(opName);
            }));
        groupMenu->addItem(opItem);
    }

    menu->addMenu(groupMenu);

    // Dimension commands — same shape as Creation.
    api::Menu* groupMenu2 = new api::Menu("Dimensions");
    for (const std::string& opName : _operationMap["Dimensions"]) {
        lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(cleanOperationName(opName).c_str());
        opItem->addCallback(
            lc::scripting::nativeCallback([self, opName]() {
                self->runOperationByName(opName);
            }));
        groupMenu2->addItem(opItem);
    }

    menu->addMenu(groupMenu2);
}

void ContextMenuManager::selectedCommands(api::Menu* menu, const std::vector<lc::entity::CADEntity_CSPtr>& selectedEntities) {
    // Phase 4 PR-8 — 5 dostrings killed: Copy, Modify-loop (via
    // runOperationByName), Select All, Select None, Invert Selection.
    MainWindow* self = _mainWindow;

    // Copy Command — was `contextmenu_op = function()
    // mainWindow:copySelectedEntities(mainWindow:cadMdiChild():selection()) end`.
    // The nested call reads the current selection at click time, not
    // menu-generation time — preserved by resolving inside the lambda.
    api::MenuItem* copyItem = new api::MenuItem("Copy");
    copyItem->addCallback(
        lc::scripting::nativeCallback([self]() {
            self->copySelectedEntities(self->cadMdiChild()->selection());
        }));
    menu->addItem(copyItem);

    // Modify commands — same shape as Creation/Dimensions in inactiveCommands.
    api::Menu* groupMenu = new api::Menu("Modify");
    for (const std::string& opName : _operationMap["Modify"]) {
        lc::ui::api::MenuItem* opItem = new lc::ui::api::MenuItem(cleanOperationName(opName).c_str());
        opItem->addCallback(
            lc::scripting::nativeCallback([self, opName]() {
                self->runOperationByName(opName);
            }));
        groupMenu->addItem(opItem);
    }
    menu->addMenu(groupMenu);

    // Select commands.
    api::Menu* selectMenu = new api::Menu("Select");
    api::MenuItem* selectAllItem = new api::MenuItem("Select All");
    api::MenuItem* selectNoneItem = new api::MenuItem("Select None");
    api::MenuItem* selectInverted = new api::MenuItem("Invert Selection");

    selectAllItem->addCallback(
        lc::scripting::nativeCallback([self]() { self->selectAll(); }));
    selectNoneItem->addCallback(
        lc::scripting::nativeCallback([self]() { self->selectNone(); }));
    selectInverted->addCallback(
        lc::scripting::nativeCallback([self]() { self->invertSelection(); }));

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

std::string ContextMenuManager::cleanTransitionName(const std::string& transitionName) const {
    std::string result;
    int n = transitionName.size();
    int last = 0;

    for (int i = 1; i < n; i++) {
        if (transitionName[i] >= 'A' && transitionName[i] <= 'Z') {
            result += transitionName.substr(last, i-last) + " ";
            last = i;
        }
    }

    result += transitionName.substr(last);
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

    // Phase 4 PR-8 — 4 `contextmenu_op` dostrings killed.  Each snap
    // callback toggles one snap flag on the CadMdiChild's SnapManager
    // and re-syncs the toolbar snap buttons.  Uses the generic
    // vector<ScriptValue> nativeCallback shape because checked callbacks
    // receive one bool arg (from MenuItem::itemToggled's `.call(toggle)`).
    MainWindow* self = _mainWindow;
    auto makeSnapCb = [self](void (viewer::manager::SnapManagerImpl::*setter)(bool)) {
        return lc::scripting::nativeCallback(
            [self, setter](const std::vector<lc::scripting::ScriptValue>& args)
                -> lc::scripting::ScriptValue {
                bool enabled = !args.empty() ? args[0].asBool() : false;
                auto sm = self->cadMdiChild()->getSnapManager();
                (sm.get()->*setter)(enabled);
                self->toolbar()->updateSnapButtons(sm);
                return lc::scripting::ScriptValue{};
            });
    };
    snapGridItem->addCheckedCallback(
        makeSnapCb(&viewer::manager::SnapManagerImpl::setGridSnappable));
    snapIntersectionItem->addCheckedCallback(
        makeSnapCb(&viewer::manager::SnapManagerImpl::setIntersectionsSnappable));
    snapMiddleItem->addCheckedCallback(
        makeSnapCb(&viewer::manager::SnapManagerImpl::setMiddleSnappable));
    snapEntityItem->addCheckedCallback(
        makeSnapCb(&viewer::manager::SnapManagerImpl::setEntitySnappable));

    snapMenu->addItem(snapGridItem);
    snapMenu->addItem(snapIntersectionItem);
    snapMenu->addItem(snapMiddleItem);
    snapMenu->addItem(snapEntityItem);
    menu->addMenu(snapMenu);
}

void ContextMenuManager::operationContextCommands(api::Menu* menu, const std::vector<lc::entity::CADEntity_CSPtr>& selectedEntities) {
    // Phase 4 PR-8 — step-transition binding + contextMenuOptions
    // dispatch, both routed through ScriptObject.  The `menu` arg to
    // the Lua-side `contextMenuOptions(op, menu)` method is passed as
    // an OpaquePtr with the api::Menu* tag; the encoder registered in
    // guibridge.cpp's luaOpenGUIBridge materializes it back into the
    // Menu userdata Lua callers expect.
    std::string entityName = _mainWindow->lastOperationName();
    lc::scripting::ScriptObject currentOp = _mainWindow->currentOperation();

    if (_transitionMap.find(entityName) != _transitionMap.end()) {
        lc::scripting::ScriptValue stepName = currentOp.getAttr("step");

        if (!stepName.isNil()) {
            std::string step = stepName.asString();

            if (_transitionMap[entityName].find(step) != _transitionMap[entityName].end()) {
                std::vector<std::string> transitions = _transitionMap[entityName][step];

                for (std::string transition : transitions) {
                    api::MenuItem* menuItem = new api::MenuItem(cleanTransitionName(transition).c_str());
                    // Native lambda replaces `contextmenu_op = function()
                    // mainWindow:currentOperation().step = '<transition>' end`.
                    MainWindow* self = _mainWindow;
                    menuItem->addCallback(
                        lc::scripting::nativeCallback([self, transition]() {
                            lc::scripting::ScriptObject op = self->currentOperation();
                            if (!op.isNil()) {
                                op.setAttr("step", lc::scripting::ScriptValue(transition));
                            }
                        }));
                    menu->addItem(menuItem);
                }
            }
        }
    }

    // Fire the operation's own `contextMenuOptions(menu)` if present.
    // The pre-refactor code did `otherOptions(currentOp, menu)` where
    // `otherOptions = currentOp["contextMenuOptions"]` was a raw
    // function LuaRef pulled off the operation table — the operation
    // received itself as arg 1 and the menu LuaRef as arg 2.  The
    // neutral form does `currentOp.callMethod("contextMenuOptions",
    // {menu-as-OpaquePtr})` — the Lua adapter's callMethod prepends
    // `self` for the `t:contextMenuOptions(menu)` call shape.
    if (!currentOp.isNil() && currentOp.hasAttr("contextMenuOptions")) {
        lc::scripting::OpaquePtr menuOpaque{menu, lc::ui::opaquetag::Menu};
        std::vector<lc::scripting::ScriptValue> args;
        args.push_back(lc::scripting::ScriptValue(menuOpaque));
        currentOp.callMethod("contextMenuOptions", args);
    }
}

void ContextMenuManager::addTransition(std::string entityName, std::string fname, std::vector<std::string> transitionList) {
    if (_transitionMap.find(entityName) == _transitionMap.end()) {
        _transitionMap[entityName] = std::map<std::string, std::vector<std::string>>();
    }

    _transitionMap[entityName][fname] = transitionList;
}
