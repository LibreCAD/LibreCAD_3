#include "contextmenumanager.h"
#include "widgets/guiAPI/menuitem.h"

using namespace lc::ui;

ContextMenuManager::ContextMenuManager(lua_State* luastate) 
    :
    _L(luastate)
{}

void ContextMenuManager::generateMenu(api::Menu* menu, std::vector<lc::entity::CADEntity_CSPtr> selectedEntities) {
    if (selectedEntities.size() > 0) {
        lc::ui::api::MenuItem* testAction = new lc::ui::api::MenuItem("Rotate");
        _L.dostring("contextmenu_op = function() run_basic_operation(RotateOperation) end");
        testAction->addCallback(_L["contextmenu_op"]);
        menu->addItem(testAction);
    }
    else {
        lc::ui::api::MenuItem* testAction2 = new lc::ui::api::MenuItem("Test");
        menu->addItem(testAction2);
        //QAction* testAction = new QAction("Line");
        //menu->addAction(testAction);
    }
}
