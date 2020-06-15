#include "lua/guibridge.h"
#include "luainterface.h"

#include "cadmdichild.h"
#include "documentcanvas.h"
#include "lcadviewer.h"
#include "widgets/luascript.h"
#include "widgets/clicommand.h"
#include "widgets/toolbar.h"
#include "widgets/guiAPI/toolbartab.h"
#include "widgets/layers.h"
#include "widgets/guiAPI/menu.h"
#include "widgets/guiAPI/menuitem.h"
#include "widgets/guiAPI/toolbarbutton.h"
#include "widgets/guiAPI/toolbargroup.h"
#include "widgets/guiAPI/dialogwidget.h"
#include "widgets/guiAPI/inputgui.h"
#include "widgets/guiAPI/textgui.h"
#include "widgets/guiAPI/buttongroupgui.h"
#include "widgets/guiAPI/buttongui.h"
#include <drawables/tempentities.h>
#include "mainwindow.h"

using namespace lc;
using namespace viewer;
using namespace ui;
using namespace widgets;
using namespace dialog;
using namespace manager;
using namespace drawable;

void luaOpenGUIBridge(lua_State *L) {
	kaguya::State state(L);
    state["gui"] = kaguya::NewTable();

	addLCBindings(L);
    addLuaGUIAPIBindings(L);
}

void addLCBindings(lua_State *L) {
	kaguya::State state(L);

	state["lc"]["CadMdiChild"].setClass(kaguya::UserdataMetatable<CadMdiChild>()
		.addStaticFunction("new", []() {
			return new CadMdiChild;
		})
		.addFunction("getSnapManager", &CadMdiChild::getSnapManager)
		.addFunction("cursor", &CadMdiChild::cursor)
		.addFunction("document", &CadMdiChild::document)
		.addFunction("saveFile", &CadMdiChild::saveFile)
		.addFunction("saveAsFile", &CadMdiChild::saveAsFile)
		.addFunction("openFile", &CadMdiChild::openFile)
		.addFunction("selection", &CadMdiChild::selection)
		.addFunction("newDocument", &CadMdiChild::newDocument)
		.addFunction("setDestroyCallback", &CadMdiChild::setDestroyCallback)
		.addFunction("tempEntities", &CadMdiChild::tempEntities)
		.addFunction("undoManager", &CadMdiChild::undoManager)
		.addFunction("viewer", &CadMdiChild::viewer)
		.addFunction("activeLayer", &CadMdiChild::activeLayer)
		.addFunction("metaInfoManager", &CadMdiChild::metaInfoManager)
		.addFunction("getFilename", &CadMdiChild::getFilename)
		.addFunction("viewerProxy", &CadMdiChild::viewerProxy)
		.addFunction("activeViewport", &CadMdiChild::activeViewport)
	);

    state["lc"]["LCADViewerProxy"].setClass(kaguya::UserdataMetatable<LCADViewerProxy>());

	state["lc"]["Cursor"].setClass(kaguya::UserdataMetatable<drawable::Cursor>()
	    .addFunction("position", &drawable::Cursor::position)
	);

	state["lc"]["LCADViewer"].setClass(kaguya::UserdataMetatable<LCADViewer>()
	    .addFunction("autoScale", &LCADViewer::autoScale)
	    .addFunction("setOperationActive", &LCADViewer::setOperationActive)
	    .addFunction("docCanvas", &LCADViewer::docCanvas)
	);

	state["lc"]["LuaInterface"].setClass(kaguya::UserdataMetatable<LuaInterface>()
	    .addFunction("pluginList", &LuaInterface::pluginList)
	    .addFunction("operation", &LuaInterface::operation)
	    .addFunction("setOperation", &LuaInterface::setOperation)
	    .addFunction("registerEvent", &LuaInterface::registerEvent)
	    .addFunction("deleteEvent", &LuaInterface::deleteEvent)
	    .addFunction("triggerEvent", &LuaInterface::triggerEvent)
        .addFunction("finishOperation", &LuaInterface::finishOperation)
	);

    state["lc"]["LuaScript"].setClass(kaguya::UserdataMetatable<widgets::LuaScript>()
        .setConstructors<widgets::LuaScript(lc::ui::MainWindow*)>()
        .addFunction("show", &widgets::LuaScript::show)
	);

	state["lc"]["DocumentCanvas"].setClass(kaguya::UserdataMetatable<DocumentCanvas>()
	    .addFunction("autoScale", &DocumentCanvas::autoScale)
		.addFunction("selectPoint", &DocumentCanvas::selectPoint)
        .addFunction("removeSelection", &DocumentCanvas::removeSelection)
        .addFunction("inverseSelection", &DocumentCanvas::inverseSelection)
        .addFunction("selectAll", &DocumentCanvas::selectAll)
	);

	state["lc"]["CliCommand"].setClass(kaguya::UserdataMetatable<widgets::CliCommand>()
	    .addFunction("addCommand", &widgets::CliCommand::addCommand)
	    .addStaticFunction("write", [](widgets::CliCommand* cliCommand, const char* message) {
	        cliCommand->write(message);
	    })
	    .addFunction("returnText", &widgets::CliCommand::returnText)
	    .addFunction("commandActive", &widgets::CliCommand::commandActive)
        .addFunction("runCommand", &widgets::CliCommand::runCommand)
        .addStaticFunction("enableCommand", [](widgets::CliCommand* cliCommand, const char* command) {
            cliCommand->enableCommand(command, true);
        })
        .addStaticFunction("disableCommand", [](widgets::CliCommand* cliCommand, const char* command) {
            cliCommand->enableCommand(command, false);
        })
        .addFunction("isCommandEnabled", &widgets::CliCommand::isCommandEnabled)
        .addFunction("availableCommands", &widgets::CliCommand::availableCommands)
        .addFunction("commandsHistory", &widgets::CliCommand::commandsHistory)
        .addFunction("clear", &widgets::CliCommand::clear)
	);

	state["lc"]["Toolbar"].setClass(kaguya::UserdataMetatable<widgets::Toolbar>()
		.addFunction("tabByName", &widgets::Toolbar::tabByName)
        .addFunction("removeGroupByName", &widgets::Toolbar::removeGroupByName)
        .addOverloadedFunctions("addTab", static_cast<api::ToolbarTab*(widgets::Toolbar::*)(const char*)>(&widgets::Toolbar::addTab),
            static_cast<void(widgets::Toolbar::*)(api::ToolbarTab*)>(&widgets::Toolbar::addTab))
        .addOverloadedFunctions("removeTab", static_cast<void(widgets::Toolbar::*)(api::ToolbarTab*)>(&widgets::Toolbar::removeTab),
            static_cast<void(widgets::Toolbar::*)(const char*)>(&widgets::Toolbar::removeTab))
        .addStaticFunction("addButton", [](widgets::Toolbar* toolbar, const char* name, const char* icon, const char* group, kaguya::LuaRef cb, const char* tooltip) {
            toolbar->addButton(name, icon, group, cb, tooltip);
        })
	);

	state["lc"]["TempEntities"].setClass(kaguya::UserdataMetatable<drawable::TempEntities>()
		.addFunction("addEntity", &drawable::TempEntities::addEntity)
		.addFunction("removeEntity", &drawable::TempEntities::removeEntity)
	);

	state["lc"]["MetaInfoManager"].setClass(kaguya::UserdataMetatable<lc::ui::MetaInfoManager>()
        .addFunction("metaInfo", &lc::ui::MetaInfoManager::metaInfo)
	);

	state["lc"]["Layers"].setClass(kaguya::UserdataMetatable<widgets::Layers>()
		.addFunction("setMdiChild", &widgets::Layers::setMdiChild)
        .addFunction("layerByName", &widgets::Layers::layerByName)
        .addOverloadedFunctions("addLayer", static_cast<void(widgets::Layers::*)(lc::meta::Layer_CSPtr)>(&widgets::Layers::addLayer),
            static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*)>(&widgets::Layers::addLayer),
            static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*, double)>(&widgets::Layers::addLayer),
            static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*, int, int, int)>(&widgets::Layers::addLayer),
            static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*, double, int, int, int)>(&widgets::Layers::addLayer),
            static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*, lc::Color)>(&widgets::Layers::addLayer),
            static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*, double, lc::Color)>(&widgets::Layers::addLayer))
        .addOverloadedFunctions("removeLayer", static_cast<void(widgets::Layers::*)(lc::meta::Layer_CSPtr)>(&widgets::Layers::removeLayer),
            static_cast<void(widgets::Layers::*)(const char*)>(&widgets::Layers::removeLayer))
        .addOverloadedFunctions("renameLayer", static_cast<void(widgets::Layers::*)(lc::meta::Layer_CSPtr, const char*)>(&widgets::Layers::renameLayer),
            static_cast<void(widgets::Layers::*)(const char*, const char*)>(&widgets::Layers::renameLayer))
        .addOverloadedFunctions("replaceLayer", static_cast<void(widgets::Layers::*)(lc::meta::Layer_CSPtr, lc::meta::Layer_CSPtr)>(&widgets::Layers::replaceLayerAPI),
            static_cast<void(widgets::Layers::*)(const char*, lc::meta::Layer_CSPtr)>(&widgets::Layers::replaceLayerAPI))
	);

	state["lc"]["SnapManager"].setClass(kaguya::UserdataMetatable<manager::SnapManagerImpl>()
	    .addFunction("setGridSnappable", &manager::SnapManagerImpl::setGridSnappable)
	    .addFunction("setIntersectionSnappable", &manager::SnapManagerImpl::setIntersectionsSnappable)
	    .addFunction("setMiddleSnappable", &manager::SnapManagerImpl::setMiddleSnappable)
	    .addFunction("setEntitySnappable", &manager::SnapManagerImpl::setEntitySnappable)
	);

    state["lc"]["MainWindow"].setClass(kaguya::UserdataMetatable<lc::ui::MainWindow>()
        .addFunction("connectMenuItem", &lc::ui::MainWindow::connectMenuItem)
        .addFunction("cliCommand", &lc::ui::MainWindow::cliCommand)
        .addFunction("cadMdiChild", &lc::ui::MainWindow::cadMdiChild)
        .addFunction("toolbar", &lc::ui::MainWindow::toolbar)
        .addFunction("layers", &lc::ui::MainWindow::layers)
        .addFunction("operationFinished", &lc::ui::MainWindow::operationFinished)
        .addFunction("findMenuItem", &lc::ui::MainWindow::findMenuItem)
        .addFunction("findMenuItemByObjectName", &lc::ui::MainWindow::findMenuItemByObjectName)
        .addFunction("menuByName", &lc::ui::MainWindow::menuByName)
        .addFunction("menuByPosition", &lc::ui::MainWindow::menuByPosition)
        .addOverloadedFunctions("addMenu", static_cast<lc::ui::api::Menu*(lc::ui::MainWindow::*)(const std::string&)>(&lc::ui::MainWindow::addMenu), static_cast<void(lc::ui::MainWindow::*)(lc::ui::api::Menu*)>(&lc::ui::MainWindow::addMenu))
        .addOverloadedFunctions("removeMenu", static_cast<void(lc::ui::MainWindow::*)(const char*)>(&lc::ui::MainWindow::removeMenu), static_cast<void(lc::ui::MainWindow::*)(int)>(&lc::ui::MainWindow::removeMenu))
        .addOverloadedFunctions("runOperation", &lc::ui::MainWindow::runOperation, [](lc::ui::MainWindow& self, kaguya::LuaRef operation) { self.runOperation(operation); })
    );
}

void addLuaGUIAPIBindings(lua_State* L) {
    kaguya::State state(L);

    state["gui"]["Menu"].setClass(kaguya::UserdataMetatable<lc::ui::api::Menu>()
        .setConstructors<lc::ui::api::Menu(const char*)>()
        .addFunction("label", &lc::ui::api::Menu::label)
        .addFunction("setLabel", &lc::ui::api::Menu::setLabel)
        .addFunction("hide", &lc::ui::api::Menu::hide)
        .addFunction("show", &lc::ui::api::Menu::show)
        .addFunction("isEnabled", &lc::ui::api::Menu::isEnabled)
        .addFunction("setEnabled", &lc::ui::api::Menu::setEnabled)
        .addFunction("position", &lc::ui::api::Menu::position)
        .addFunction("setPosition", &lc::ui::api::Menu::setPosition)
        .addFunction("remove", &lc::ui::api::Menu::remove)
        .addFunction("itemByName", &lc::ui::api::Menu::itemByName)
        .addFunction("itemByPosition", &lc::ui::api::Menu::itemByPosition)
        .addFunction("menuByName", &lc::ui::api::Menu::menuByName)
        .addFunction("menuByPosition", &lc::ui::api::Menu::menuByPosition)
        .addOverloadedFunctions("addItem", static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::MenuItem*)>(&lc::ui::api::Menu::addItem), static_cast<lc::ui::api::MenuItem * (lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::addItem), static_cast<lc::ui::api::MenuItem * (lc::ui::api::Menu::*)(const char*, kaguya::LuaRef)>(&lc::ui::api::Menu::addItem))
        .addOverloadedFunctions("removeItem", static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::MenuItem*)>(&lc::ui::api::Menu::removeItem), static_cast<void(lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::removeItem))
        .addOverloadedFunctions("removeMenu", static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::Menu*)>(&lc::ui::api::Menu::removeMenu), static_cast<void(lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::removeMenu))
        .addOverloadedFunctions("addMenu", static_cast<lc::ui::api::Menu * (lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::addMenu), static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::Menu*)>(&lc::ui::api::Menu::addMenu))
    );

    state["gui"]["MenuItem"].setClass(kaguya::UserdataMetatable<lc::ui::api::MenuItem>()
        .setConstructors<lc::ui::api::MenuItem(const char*), lc::ui::api::MenuItem(const char*, kaguya::LuaRef)>()
        .addFunction("label", &lc::ui::api::MenuItem::label)
        .addFunction("setLabel", &lc::ui::api::MenuItem::setLabel)
        .addFunction("hide", &lc::ui::api::MenuItem::hide)
        .addFunction("show", &lc::ui::api::MenuItem::show)
        .addFunction("isEnabled", &lc::ui::api::MenuItem::isEnabled)
        .addFunction("setEnabled", &lc::ui::api::MenuItem::setEnabled)
        .addFunction("position", &lc::ui::api::MenuItem::position)
        .addFunction("setPosition", &lc::ui::api::MenuItem::setPosition)
        .addFunction("remove", &lc::ui::api::MenuItem::remove)
        .addFunction("removeCallback", &lc::ui::api::MenuItem::removeCallback)
        .addOverloadedFunctions("addCallback", static_cast<void(lc::ui::api::MenuItem::*)(kaguya::LuaRef)>(&lc::ui::api::MenuItem::addCallback), static_cast<void(lc::ui::api::MenuItem::*)(const char*, kaguya::LuaRef)>(&lc::ui::api::MenuItem::addCallback))
    );

    state["gui"]["ToolbarTab"].setClass(kaguya::UserdataMetatable<api::ToolbarTab>()
        .setConstructors<api::ToolbarTab(const char*)>()
        .addFunction("addButton", &api::ToolbarTab::addButton)
        .addFunction("buttonByText", &api::ToolbarTab::buttonByText)
        .addFunction("groupByName", &api::ToolbarTab::groupByName)
        .addFunction("removeButton", &api::ToolbarTab::removeButton)
        .addFunction("label", &api::ToolbarTab::label)
        .addFunction("setLabel", &api::ToolbarTab::setLabel)
        .addFunction("getAllGroups", &api::ToolbarTab::getAllGroups)
        .addFunction("remove", &api::ToolbarTab::remove)
        .addOverloadedFunctions("enable", [](lc::ui::api::ToolbarTab& self) { self.setEnabled(true); })
        .addOverloadedFunctions("disable", [](lc::ui::api::ToolbarTab& self) { self.setEnabled(false); })
        .addOverloadedFunctions("removeGroup", static_cast<void(api::ToolbarTab::*)(lc::ui::api::ToolbarGroup*)>(&api::ToolbarTab::removeGroup),
            static_cast<void(api::ToolbarTab::*)(const char*)>(&api::ToolbarTab::removeGroup))
        .addOverloadedFunctions("addGroup", static_cast<void(api::ToolbarTab::*)(api::ToolbarGroup*)>(&api::ToolbarTab::addGroup),
            static_cast<api::ToolbarGroup * (api::ToolbarTab::*)(const char*)>(&api::ToolbarTab::addGroup))
    );

    state["gui"]["ToolbarButton"].setClass(kaguya::UserdataMetatable<lc::ui::api::ToolbarButton>()
        .setConstructors<lc::ui::api::ToolbarButton(const char*, const char*),
        lc::ui::api::ToolbarButton(const char*, const char*, kaguya::LuaRef),
        lc::ui::api::ToolbarButton(const char*, const char*, const char*),
        lc::ui::api::ToolbarButton(const char*, const char*, kaguya::LuaRef, const char*),
        lc::ui::api::ToolbarButton(const char*, const char*, const char*, bool),
        lc::ui::api::ToolbarButton(const char*, const char*, kaguya::LuaRef, const char*, bool)>()
        .addFunction("label", &lc::ui::api::ToolbarButton::label)
        .addFunction("setLabel", &lc::ui::api::ToolbarButton::setLabel)
        .addFunction("setTooltip", &lc::ui::api::ToolbarButton::setTooltip)
        .addFunction("removeCallback", &lc::ui::api::ToolbarButton::removeCallback)
        .addFunction("isEnabled", &lc::ui::api::ToolbarButton::isEnabled)
        .addFunction("hide", &lc::ui::api::ToolbarButton::hide)
        .addFunction("show", &lc::ui::api::ToolbarButton::show)
        .addFunction("remove", &lc::ui::api::ToolbarButton::remove)
        .addOverloadedFunctions("enable", [](lc::ui::api::ToolbarButton& self) { self.setEnabled(true); })
        .addOverloadedFunctions("disable", [](lc::ui::api::ToolbarButton& self) { self.setEnabled(false); })
        .addOverloadedFunctions("addCallback", static_cast<void(lc::ui::api::ToolbarButton::*)(kaguya::LuaRef)>(&lc::ui::api::ToolbarButton::addCallback),
            static_cast<void(lc::ui::api::ToolbarButton::*)(const char*,kaguya::LuaRef)>(&lc::ui::api::ToolbarButton::addCallback))
    );

    state["gui"]["ToolbarGroup"].setClass(kaguya::UserdataMetatable<lc::ui::api::ToolbarGroup>()
        .setConstructors<lc::ui::api::ToolbarGroup(const char*), lc::ui::api::ToolbarGroup(const char*, int)>()
        .addFunction("label", &lc::ui::api::ToolbarGroup::label)
        .addFunction("setLabel", &lc::ui::api::ToolbarGroup::setLabel)
        .addFunction("buttonByName", &lc::ui::api::ToolbarGroup::buttonByName)
        .addFunction("getAllButtons", &lc::ui::api::ToolbarGroup::getAllButtons)
        .addFunction("hide", &lc::ui::api::ToolbarGroup::hide)
        .addFunction("show", &lc::ui::api::ToolbarGroup::show)
        .addFunction("remove", &lc::ui::api::ToolbarGroup::remove)
        .addFunction("setWidth", &lc::ui::api::ToolbarGroup::setWidth)
        .addOverloadedFunctions("enable", [](lc::ui::api::ToolbarGroup& self) { self.setEnabled(true); })
        .addOverloadedFunctions("disable", [](lc::ui::api::ToolbarGroup& self) { self.setEnabled(false); })
        .addOverloadedFunctions("addButton", static_cast<void(lc::ui::api::ToolbarGroup::*)(lc::ui::api::ToolbarButton*)>(&lc::ui::api::ToolbarGroup::addButton),
            static_cast<lc::ui::api::ToolbarButton*(lc::ui::api::ToolbarGroup::*)(const char*, const char*)>(&lc::ui::api::ToolbarGroup::addButton),
            static_cast<lc::ui::api::ToolbarButton*(lc::ui::api::ToolbarGroup::*)(const char*, const char*, kaguya::LuaRef)>(&lc::ui::api::ToolbarGroup::addButton))
        .addOverloadedFunctions("removeButton", static_cast<void(lc::ui::api::ToolbarGroup::*)(lc::ui::api::ToolbarButton*)>(&lc::ui::api::ToolbarGroup::removeButton),
            static_cast<void(lc::ui::api::ToolbarGroup::*)(const char*)>(&lc::ui::api::ToolbarGroup::removeButton))
    );

    state["gui"]["DialogWidget"].setClass(kaguya::UserdataMetatable<lc::ui::api::DialogWidget>()
        .setConstructors<lc::ui::api::DialogWidget(const std::string&)>()
        .addFunction("inputWidgets", &lc::ui::api::DialogWidget::inputWidgets)
        .addOverloadedFunctions("enable", [](lc::ui::api::DialogWidget& self) { self.setEnabled(true); })
        .addOverloadedFunctions("disable", [](lc::ui::api::DialogWidget& self) { self.setEnabled(false); })
        .addOverloadedFunctions("addWidget", static_cast<void(lc::ui::api::DialogWidget::*)(lc::ui::api::InputGUI*)>(&lc::ui::api::DialogWidget::addWidget),
            static_cast<void(lc::ui::api::DialogWidget::*)(lc::ui::api::ButtonGUI*)>(&lc::ui::api::DialogWidget::addWidget))
    );

    state["gui"]["InputGUI"].setClass(kaguya::UserdataMetatable<lc::ui::api::InputGUI>()
        .addFunction("label", &lc::ui::api::InputGUI::label)
        .addFunction("setLabel", &lc::ui::api::InputGUI::setLabel)
        .addOverloadedFunctions("enable", [](lc::ui::api::InputGUI& self) { self.setEnabled(true); })
        .addOverloadedFunctions("disable", [](lc::ui::api::InputGUI& self) { self.setEnabled(false); })
    );

    state["gui"]["TextGUI"].setClass(kaguya::UserdataMetatable<lc::ui::api::TextGUI, lc::ui::api::InputGUI>()
        .setConstructors<lc::ui::api::TextGUI(std::string)>()
        .addFunction("text", &lc::ui::api::TextGUI::text)
        .addFunction("setText", &lc::ui::api::TextGUI::setText)
        .addFunction("addFinishCallback", &lc::ui::api::TextGUI::addFinishCallback)
        .addFunction("addOnChangeCallback", &lc::ui::api::TextGUI::addOnChangeCallback)
    );

    state["gui"]["ButtonGUI"].setClass(kaguya::UserdataMetatable<lc::ui::api::ButtonGUI>()
        .setConstructors<lc::ui::api::ButtonGUI(std::string)>()
        .addFunction("label", &lc::ui::api::ButtonGUI::label)
        .addFunction("setLabel", &lc::ui::api::ButtonGUI::setLabel)
        .addFunction("addCallback", &lc::ui::api::ButtonGUI::addCallback)
    );

    state["gui"]["ButtonGroupGUI"].setClass(kaguya::UserdataMetatable<lc::ui::api::ButtonGroupGUI, lc::ui::api::InputGUI>()
        .setConstructors<lc::ui::api::ButtonGroupGUI(std::string)>()
        .addFunction("addButton", &lc::ui::api::ButtonGroupGUI::addButton)
    );
}
