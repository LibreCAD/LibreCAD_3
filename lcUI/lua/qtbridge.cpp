#include "lua/qtbridge.h"
#include "luaqobject.h"
#include "luainterface.h"

#include "cadmdichild.h"
#include "documentcanvas.h"
#include "lcadviewer.h"
#include "widgets/luascript.h"
#include "widgets/clicommand.h"
#include "widgets/toolbar.h"
#include "widgets/toolbartab.h"
#include "widgets/layers.h"
#include "widgets/guiAPI/menu.h"
#include "widgets/guiAPI/menuitem.h"
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
	);

	state["lc"]["Toolbar"].setClass(kaguya::UserdataMetatable<widgets::Toolbar, QDockWidget>()
		.addFunction("addTab", &widgets::Toolbar::addTab)
		.addFunction("removeTab", &widgets::Toolbar::removeTab)
		.addFunction("tabByName", &widgets::Toolbar::tabByName)
        .addFunction("addButton", &widgets::Toolbar::addButton)
        .addFunction("addCheckableButton", &widgets::Toolbar::addCheckableButton)
        .addFunction("removeGroupByName", &widgets::Toolbar::removeGroupByName)
	);

	state["lc"]["ToolbarTab"].setClass(kaguya::UserdataMetatable<widgets::ToolbarTab, QWidget>()
        .addStaticFunction("new", []() {
            return new ToolbarTab;
        })
        .addFunction("addButton", &widgets::ToolbarTab::addButton)
	    .addFunction("addWidget", &widgets::ToolbarTab::addWidget)
		.addFunction("addGroup", &widgets::ToolbarTab::addGroup)
		.addFunction("buttonByText", &widgets::ToolbarTab::buttonByText)
		.addFunction("groupByName", &widgets::ToolbarTab::groupByName)
		.addFunction("removeGroup", &widgets::ToolbarTab::removeGroup)
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
        .addFunction("operationFinished", &lc::ui::MainWindow::operationFinished)
        .addFunction("getMenu", &lc::ui::MainWindow::getMenu)
        .addOverloadedFunctions("addMenu", static_cast<lc::ui::api::Menu*(lc::ui::MainWindow::*)(const std::string&)>(&lc::ui::MainWindow::addMenu), static_cast<void(lc::ui::MainWindow::*)(lc::ui::api::Menu*)>(&lc::ui::MainWindow::addMenu))
        .addOverloadedFunctions("runOperation", &lc::ui::MainWindow::runOperation, [](lc::ui::MainWindow& self, kaguya::LuaRef operation) { self.runOperation(operation); })
    );
}

void addLuaGUIAPIBindings(lua_State* L) {
    kaguya::State state(L);

    state["gui"]["Menu"].setClass(kaguya::UserdataMetatable<lc::ui::api::Menu>()
        .setConstructors<lc::ui::api::Menu(const char*)>()
        .addFunction("addItem", &lc::ui::api::Menu::addItem)
        .addFunction("getLabel", &lc::ui::api::Menu::getLabel)
    );

    state["gui"]["MenuItem"].setClass(kaguya::UserdataMetatable<lc::ui::api::MenuItem>()
        .setConstructors<lc::ui::api::MenuItem(const char*), lc::ui::api::MenuItem(const char*, kaguya::LuaRef)>()
    );
}

void addQtMetaTypes() {
	qRegisterMetaType<lc::geo::Coordinate>();
}
