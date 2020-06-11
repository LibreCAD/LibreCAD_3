#include "lua/qtbridge.h"
#include "luaqobject.h"
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
#include "dialogs/linepatternmanager.h"
#include <drawables/tempentities.h>
#include "widgets/linewidthselect.h"
#include "widgets/colorselect.h"
#include "mainwindow.h"

using namespace lc;
using namespace viewer;
using namespace ui;
using namespace widgets;
using namespace dialog;
using namespace manager;
using namespace drawable;

void luaOpenQtBridge(lua_State *L) {
	kaguya::State state(L);
	state["qt"] = kaguya::NewTable();
    state["gui"] = kaguya::NewTable();

	addQtBaseBindings(L);
	addQtWindowBindings(L);
	addQtLayoutBindings(L);
	addQtWidgetsBindings(L);
	addLCBindings(L);
    addLuaGUIAPIBindings(L);
	addQtMetaTypes();
}

void addQtBaseBindings(lua_State *L) {
	kaguya::State state(L);

	state["qt"]["loadUi"].setFunction([](const char* fileName) { return static_cast<QMainWindow*>(LuaInterface::loadUiFile(fileName)); });
	state["qt"]["QObject"].setClass(kaguya::UserdataMetatable<QObject>()
		.addStaticFunction("findChild", [](QObject* object, std::string name) {
			return LuaQObject::findChild(object, name);
		})
		.addStaticFunction("tr", [](const char* text) {
		    return QObject::tr(text);
		})
	);

	state["qt"]["QWidget"].setClass(kaguya::UserdataMetatable<QWidget, QObject>()
		.addFunction("addAction", &QWidget::addAction)
		.addFunction("setWindowTitle", &QWidget::setWindowTitle)
		.addFunction("show", &QWidget::show)
		.addFunction("showMaximized", &QWidget::showMaximized)
        .addFunction("setToolTip", &QWidget::setToolTip)
        .addFunction("setFocus", static_cast<void (QWidget::*)()>(&QMainWindow::setFocus))
	);

	state["qt"]["QString"].setClass(kaguya::UserdataMetatable<QString>()
	    .setConstructors<QString(const char*)>()
		.addFunction("isEmpty", &QString::isEmpty)
		.addFunction("toStdString", &QString::toStdString)
	);

	state["qt"]["QAction"].setClass(kaguya::UserdataMetatable<QAction, QObject>()
		.addFunction("setText", &QAction::setText)
		.addFunction("setIcon", &QAction::setIcon)
	);

	state["qt"]["QIcon"].setClass(kaguya::UserdataMetatable<QIcon>()
	    .setConstructors<QIcon(QString)>()
	);

	state["qt"]["QSize"].setClass(kaguya::UserdataMetatable<QSize>()
		.setConstructors<QSize(int, int)>()
	);
}

void addQtWindowBindings(lua_State *L) {
	kaguya::State state(L);

	state["qt"]["QMainWindow"].setClass(kaguya::UserdataMetatable<QMainWindow, QWidget>()
		.setConstructors<QMainWindow()>()
		.addFunction("addDockWidget", static_cast<void (QMainWindow::*)(Qt::DockWidgetArea, QDockWidget *)>(&QMainWindow::addDockWidget))
		.addFunction("menuBar", &QMainWindow::menuBar)
		.addFunction("centralWidget", &QMainWindow::centralWidget)
		.addFunction("setCentralWidget", &QMainWindow::setCentralWidget)
		.addFunction("setUnifiedTitleAndToolBarOnMac", &QMainWindow::setUnifiedTitleAndToolBarOnMac)
	);

	state["qt"]["QMenuBar"].setClass(kaguya::UserdataMetatable<QMenuBar, QWidget>()
        .addFunction("clear", &QMenuBar::clear)
        .addFunction("addMenu", static_cast<QAction* (QMenuBar::*)(QMenu*)>(&QMenuBar::addMenu))
        .addFunction("addMenuStr", static_cast<QMenu* (QMenuBar::*)(const QString&)>(&QMenuBar::addMenu))
	);

	state["qt"]["QMenu"].setClass(kaguya::UserdataMetatable<QMenu, QWidget>()
		.setConstructors<QMenu()>() //return new QMenu(); before
	    .addFunction("setTitle", &QMenu::setTitle)
	    .addFunction("addActionStr", static_cast<QAction* (QMenu::*)(const QString&)>(&QMenu::addAction))
	);

	state["qt"]["QFrame"].setClass(kaguya::UserdataMetatable<QFrame, QWidget>());

	state["qt"]["QAbstractScrollArea"].setClass(kaguya::UserdataMetatable<QAbstractScrollArea, QFrame>()
		.addFunction("setHorizontalScrollBarPolicy", &QAbstractScrollArea::setHorizontalScrollBarPolicy)
		.addFunction("setVerticalScrollBarPolicy", &QAbstractScrollArea::setVerticalScrollBarPolicy)
	);

	state["qt"]["QMdiArea"].setClass(kaguya::UserdataMetatable<QMdiArea, QAbstractScrollArea>()
		.addFunction("activeSubWindow", &QMdiArea::activeSubWindow)
	    .addStaticFunction("addSubWindow", [](QMdiArea* mdiArea, QWidget* subWindow) {
		    return mdiArea->addSubWindow(subWindow);
	    })
		.addFunction("subWindowList", &QMdiArea::subWindowList)
	);

	state["qt"]["QMdiSubWindow"].setClass(kaguya::UserdataMetatable<QMdiSubWindow, QWidget>()
	    .addFunction("widget", &QMdiSubWindow::widget)
	);

	state["qt"]["QDialog"].setClass(kaguya::UserdataMetatable<QDialog, QWidget>());

	state["qt"]["QFileDialog"].setClass(kaguya::UserdataMetatable<QFileDialog, QDialog>()
		.addStaticFunction("getOpenFileName", [](QWidget *parent, QString& caption, QString& dir, QString& filter) {
			return QFileDialog::getOpenFileName(parent, caption, dir, filter);
		})
		.addStaticFunction("getSaveFileName", [](QWidget *parent, QString& caption, QString& dir, QString& filter) {
			return QFileDialog::getSaveFileName(parent, caption, dir, filter);
		})
	);

	state["qt"]["QDockWidget"].setClass(kaguya::UserdataMetatable<QDockWidget, QWidget>());
}

void addQtLayoutBindings(lua_State *L) {
	kaguya::State state(L);

	state["qt"]["QLayout"].setClass(kaguya::UserdataMetatable<QLayout, QObject>()
		.addFunction("addWidget", &QLayout::addWidget)
		.addFunction("count", &QLayout::count)
	);

	state["qt"]["QBoxLayout"].setClass(kaguya::UserdataMetatable<QBoxLayout, QLayout>());

	state["qt"]["QHBoxLayout"].setClass(kaguya::UserdataMetatable<QHBoxLayout, QBoxLayout>()
		.setConstructors<QHBoxLayout()>()
	);

	state["qt"]["QVBoxLayout"].setClass(kaguya::UserdataMetatable<QVBoxLayout, QBoxLayout>()
		.setConstructors<QVBoxLayout()>()
	);

	state["qt"]["QGroupBox"].setClass(kaguya::UserdataMetatable<QGroupBox, QWidget>());

}

void addQtWidgetsBindings(lua_State *L) {
	kaguya::State state(L);

	state["qt"]["QAbstractButton"].setClass(kaguya::UserdataMetatable<QAbstractButton, QWidget>()
		.addFunction("setIcon", &QAbstractButton::setIcon)
		.addFunction("setIconSize", &QAbstractButton::setIconSize)
		.addFunction("setCheckable", &QAbstractButton::setCheckable)
	);

	state["qt"]["QComboBox"].setClass(kaguya::UserdataMetatable<QComboBox, QWidget>());

	state["qt"]["QPushButton"].setClass(kaguya::UserdataMetatable<QPushButton, QAbstractButton>()
		.addStaticFunction("new", [](const char* name) {
			return new QPushButton(name);
		})
		.addFunction("setFlat", &QPushButton::setFlat)
		.addFunction("setMenu", &QPushButton::setMenu)
	);

}

void addLCBindings(lua_State *L) {
	kaguya::State state(L);

	state["lc"]["CadMdiChild"].setClass(kaguya::UserdataMetatable<CadMdiChild, QWidget>()
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

    state["lc"]["LCADViewerProxy"].setClass(kaguya::UserdataMetatable<LCADViewerProxy, QObject>());

	state["lc"]["Cursor"].setClass(kaguya::UserdataMetatable<drawable::Cursor>()
	    .addFunction("position", &drawable::Cursor::position)
	);

	state["lc"]["LCADViewer"].setClass(kaguya::UserdataMetatable<LCADViewer, QWidget>()
	    .addFunction("autoScale", &LCADViewer::autoScale)
	    .addFunction("setOperationActive", &LCADViewer::setOperationActive)
	    .addFunction("docCanvas", &LCADViewer::docCanvas)
	);

	state["lc"]["LuaInterface"].setClass(kaguya::UserdataMetatable<LuaInterface>()
	    .addFunction("luaConnect", &LuaInterface::luaConnect)
	    .addFunction("connect", &LuaInterface::qtConnect)
	    .addFunction("pluginList", &LuaInterface::pluginList)
	    .addFunction("operation", &LuaInterface::operation)
	    .addFunction("setOperation", &LuaInterface::setOperation)
	    .addFunction("registerEvent", &LuaInterface::registerEvent)
	    .addFunction("deleteEvent", &LuaInterface::deleteEvent)
	    .addFunction("triggerEvent", &LuaInterface::triggerEvent)
        .addFunction("finishOperation", &LuaInterface::finishOperation)
	);

	state["lc"]["LuaScript"].setClass(kaguya::UserdataMetatable<widgets::LuaScript, QWidget>()
	    .setConstructors<widgets::LuaScript(CadMdiChild*, widgets::CliCommand*)>()
	);

	state["lc"]["DocumentCanvas"].setClass(kaguya::UserdataMetatable<DocumentCanvas>()
	    .addFunction("autoScale", &DocumentCanvas::autoScale)
		.addFunction("selectPoint", &DocumentCanvas::selectPoint)
        .addFunction("removeSelection", &DocumentCanvas::removeSelection)
        .addFunction("inverseSelection", &DocumentCanvas::inverseSelection)
        .addFunction("selectAll", &DocumentCanvas::selectAll)
	);

	state["lc"]["CliCommand"].setClass(kaguya::UserdataMetatable<widgets::CliCommand, QDockWidget>()
	    .setConstructors<widgets::CliCommand(), widgets::CliCommand(QWidget*)>()
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
        .addFunction("getAvailableCommands", &widgets::CliCommand::getAvailableCommands)
        .addFunction("getCommandsHistory", &widgets::CliCommand::getCommandsHistory)
        .addFunction("clear", &widgets::CliCommand::clear)
	);

	state["lc"]["Toolbar"].setClass(kaguya::UserdataMetatable<widgets::Toolbar, QDockWidget>()
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

	state["lc"]["Layers"].setClass(kaguya::UserdataMetatable<widgets::Layers, QDockWidget>()
	    .setConstructors<widgets::Layers()>() //return new widgets::Layers() before
		.addFunction("setMdiChild", &widgets::Layers::setMdiChild)
        .addFunction("getLayer", &widgets::Layers::getLayer)
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

	state["lc"]["LinePatternManager"].setClass(kaguya::UserdataMetatable<dialog::LinePatternManager, QDialog>()
	    .addStaticFunction("new", [](lc::storage::Document_SPtr document) {
 		    return new dialog::LinePatternManager(document);
        })
	    .addFunction("setDocument", &dialog::LinePatternManager::setDocument)
	);

	state["lc"]["LinePatternSelect"].setClass(kaguya::UserdataMetatable<lc::ui::widgets::LinePatternSelect, QComboBox>()
	    .addStaticFunction("new", [](lc::storage::Document_SPtr document, QWidget* parent, bool showByLayer, bool showByBlock) {
	  	    return new widgets::LinePatternSelect(document, parent, showByLayer, showByBlock);
	    })
		.addFunction("setMdiChild", &lc::ui::widgets::LinePatternSelect::setMdiChild)
	);

	state["lc"]["LineWidthSelect"].setClass(kaguya::UserdataMetatable<lc::ui::widgets::LineWidthSelect, QComboBox>()
		.addStaticFunction("new", [](MetaInfoManager_SPtr metaInfoManager, QWidget* parent, bool showByLayer, bool showByBlock) {
			return new widgets::LineWidthSelect(metaInfoManager, parent, showByLayer, showByBlock);
		})
	    .addFunction("setMetaInfoManager", &lc::ui::widgets::LineWidthSelect::setMetaInfoManager)
	);

	state["lc"]["ColorSelect"].setClass(kaguya::UserdataMetatable<lc::ui::widgets::ColorSelect, QComboBox>()
		.addStaticFunction("new", [](MetaInfoManager_SPtr metaInfoManager, QWidget* parent, bool showByLayer, bool showByBlock) {
			return new widgets::ColorSelect(metaInfoManager, parent, showByLayer, showByBlock);
		})
	    .addFunction("setMetaInfoManager", &lc::ui::widgets::ColorSelect::setMetaInfoManager)
	);

    state["lc"]["MainWindow"].setClass(kaguya::UserdataMetatable<lc::ui::MainWindow>()
        .addFunction("connectMenuItem", &lc::ui::MainWindow::connectMenuItem)
        .addFunction("getCliCommand", &lc::ui::MainWindow::getCliCommand)
        .addFunction("getCadMdiChild", &lc::ui::MainWindow::getCadMdiChild)
        .addFunction("getToolbar", &lc::ui::MainWindow::getToolbar)
        .addFunction("getLayers", &lc::ui::MainWindow::getLayers)
        .addFunction("operationFinished", &lc::ui::MainWindow::operationFinished)
        .addFunction("findMenuItem", &lc::ui::MainWindow::findMenuItem)
        .addFunction("findMenuItemByObjectName", &lc::ui::MainWindow::findMenuItemByObjectName)
        .addOverloadedFunctions("addMenu", static_cast<lc::ui::api::Menu*(lc::ui::MainWindow::*)(const std::string&)>(&lc::ui::MainWindow::addMenu), static_cast<void(lc::ui::MainWindow::*)(lc::ui::api::Menu*)>(&lc::ui::MainWindow::addMenu))
        .addOverloadedFunctions("getMenu", static_cast<lc::ui::api::Menu* (lc::ui::MainWindow::*)(const std::string&)>(&lc::ui::MainWindow::getMenu), static_cast<lc::ui::api::Menu* (lc::ui::MainWindow::*)(int)>(&lc::ui::MainWindow::getMenu))
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
        .addOverloadedFunctions("addItem", static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::MenuItem*)>(&lc::ui::api::Menu::addItem), static_cast<lc::ui::api::MenuItem * (lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::addItem), static_cast<lc::ui::api::MenuItem * (lc::ui::api::Menu::*)(const char*, kaguya::LuaRef)>(&lc::ui::api::Menu::addItem))
        .addOverloadedFunctions("removeItem", static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::MenuItem*)>(&lc::ui::api::Menu::removeItem), static_cast<void(lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::removeItem))
        .addOverloadedFunctions("removeMenu", static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::Menu*)>(&lc::ui::api::Menu::removeMenu), static_cast<void(lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::removeMenu))
        .addOverloadedFunctions("addMenu", static_cast<lc::ui::api::Menu * (lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::addMenu), static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::Menu*)>(&lc::ui::api::Menu::addMenu))
        .addOverloadedFunctions("getItem", static_cast<lc::ui::api::MenuItem * (lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::getItem), static_cast<lc::ui::api::MenuItem * (lc::ui::api::Menu::*)(int)>(&lc::ui::api::Menu::getItem))
        .addOverloadedFunctions("getMenu", static_cast<lc::ui::api::Menu * (lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::getMenu), static_cast<lc::ui::api::Menu * (lc::ui::api::Menu::*)(int)>(&lc::ui::api::Menu::getMenu))
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

    state["gui"]["ToolbarTab"].setClass(kaguya::UserdataMetatable<api::ToolbarTab, QWidget>()
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
        .addOverloadedFunctions("enable", [](lc::ui::api::ToolbarGroup& self) { self.setEnabled(true); })
        .addOverloadedFunctions("disable", [](lc::ui::api::ToolbarGroup& self) { self.setEnabled(false); })
        .addOverloadedFunctions("addButton", static_cast<void(lc::ui::api::ToolbarGroup::*)(lc::ui::api::ToolbarButton*)>(&lc::ui::api::ToolbarGroup::addButton),
            static_cast<lc::ui::api::ToolbarButton*(lc::ui::api::ToolbarGroup::*)(const char*, const char*)>(&lc::ui::api::ToolbarGroup::addButton),
            static_cast<lc::ui::api::ToolbarButton*(lc::ui::api::ToolbarGroup::*)(const char*, const char*, kaguya::LuaRef)>(&lc::ui::api::ToolbarGroup::addButton))
        .addOverloadedFunctions("removeButton", static_cast<void(lc::ui::api::ToolbarGroup::*)(lc::ui::api::ToolbarButton*)>(&lc::ui::api::ToolbarGroup::removeButton),
            static_cast<void(lc::ui::api::ToolbarGroup::*)(const char*)>(&lc::ui::api::ToolbarGroup::removeButton))
    );
}

void addQtMetaTypes() {
	qRegisterMetaType<lc::geo::Coordinate>();
}
