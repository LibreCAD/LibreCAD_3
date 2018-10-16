#include "lua/qtbridge.h"
#include "luaqobject.h"
#include "luainterface.h"

using namespace lc;
using namespace viewer;
using namespace ui;
using namespace widgets;
using namespace dialog;
using namespace manager;
using namespace drawable;

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
	LUA_USING_LIST_TYPE(std::vector)
}

void luaOpenQtBridge(lua_State *L) {	
	addQtBaseBindings(L);
	addQtWindowBindings(L);
	addQtLayoutBindings(L);
	addQtWidgetsBindings(L);
	addLCBindings(L);
	addQtMetaTypes();
}

void addQtBaseBindings(lua_State *L) {
	LuaIntf::LuaBinding(L)

	.beginModule("qt")
		.addFunction("loadUi", &LuaInterface::loadUiFile)
		
		.beginClass<QObject>("QObject")
			.addConstructor(LUA_ARGS())
			.addFunction("findChild", [](QObject* object, std::string name) {
				return LuaQObject::findChild(object, name);
			})
			.addStaticFunction("tr", &QObject::tr, LUA_ARGS(const char*, LuaIntf::_opt<const char*>, LuaIntf::_opt<int>))
		.endClass()

		.beginExtendClass<QWidget, QObject>("QWidget")
			.addConstructor(LUA_ARGS())
			.addFunction("addAction", &QWidget::addAction)
			.addFunction("setWindowTitle", &QWidget::setWindowTitle)
			.addFunction("show", &QWidget::show)
			.addFunction("showMaximized", &QWidget::showMaximized)
		.endClass()

		.beginClass<QString>("QString")
			.addConstructor(LUA_ARGS(const char*))
			.addFunction("isEmpty", &QString::isEmpty)
			.addFunction("toStdString", &QString::toStdString)
		.endClass()

		.beginExtendClass<QAction, QObject>("QAction")
			.addConstructor(LUA_ARGS(LuaIntf::_opt<QObject*>))
			.addFunction("setText", &QAction::setText)
			.addFunction("setIcon", &QAction::setIcon)
		.endClass()

		.beginClass<QIcon>("QIcon")
			.addConstructor(LUA_ARGS(QString))
		.endClass()

		.beginClass<QSize>("QSize")
			.addConstructor(LUA_ARGS(int, int))
		.endClass()
	.endModule();
}

void addQtWindowBindings(lua_State *L) {
	LuaIntf::LuaBinding(L)

	.beginModule("qt")
		.beginExtendClass<QMainWindow, QWidget>("QMainWindow")
			.addConstructor(LUA_ARGS())
			.addFunction("addDockWidget", static_cast<void (QMainWindow::*)(Qt::DockWidgetArea, QDockWidget *)>(&QMainWindow::addDockWidget))
			.addFunction("menuBar", &QMainWindow::menuBar)
			.addFunction("centralWidget", &QMainWindow::centralWidget)
			.addFunction("setCentralWidget", &QMainWindow::setCentralWidget)
			.addFunction("setUnifiedTitleAndToolBarOnMac", &QMainWindow::setUnifiedTitleAndToolBarOnMac)
		.endClass()

		.beginExtendClass<QMenuBar, QWidget>("QMenuBar")
			.addFunction("clear", &QMenuBar::clear)
			.addFunction("addMenu", static_cast<QAction* (QMenuBar::*)(QMenu*)>(&QMenuBar::addMenu))
			.addFunction("addMenuStr", static_cast<QMenu* (QMenuBar::*)(const QString&)>(&QMenuBar::addMenu))
		.endClass()

		.beginExtendClass<QMenu, QWidget>("QMenu")
			.addFactory([]() {
				return new QMenu();
			})
			.addFunction("setTitle", &QMenu::setTitle)
			.addFunction("addActionStr", static_cast<QAction* (QMenu::*)(const QString&)>(&QMenu::addAction))
		.endClass()

		.beginExtendClass<QFrame, QWidget>("QFrame")
		.endClass()

		.beginExtendClass<QAbstractScrollArea, QFrame>("QAbstractScrollArea")
			.addFunction("setHorizontalScrollBarPolicy", &QAbstractScrollArea::setHorizontalScrollBarPolicy)
			.addFunction("setVerticalScrollBarPolicy", &QAbstractScrollArea::setVerticalScrollBarPolicy)
		.endClass()

		.beginExtendClass<QMdiArea, QAbstractScrollArea>("QMdiArea")
			.addFunction("activeSubWindow", &QMdiArea::activeSubWindow)
			.addFunction("addSubWindow", [](QMdiArea* mdiArea, QWidget* subWindow) {
				return mdiArea->addSubWindow(subWindow);
			})
			.addFunction("subWindowList", &QMdiArea::subWindowList)
		.endClass()

		.beginExtendClass<QMdiSubWindow, QWidget>("QMdiSubWindow")
			.addFunction("widget", &QMdiSubWindow::widget)
		.endClass()

		.beginExtendClass<QDialog, QWidget>("QDialog")
		.endClass()

		.beginExtendClass<QFileDialog, QDialog>("QFileDialog")
			.addStaticFunction("getOpenFileName", [](QWidget *parent, QString& caption, QString& dir, QString& filter) {
				return QFileDialog::getOpenFileName(parent, caption, dir, filter);
			})
			.addStaticFunction("getSaveFileName", [](QWidget *parent, QString& caption, QString& dir, QString& filter) {
				return QFileDialog::getSaveFileName(parent, caption, dir, filter);
			})
		.endClass()

		.beginExtendClass<QDockWidget, QWidget>("QDockWidget")
		.endClass()
	.endModule();
}

void addQtLayoutBindings(lua_State *L) {
	LuaIntf::LuaBinding(L)

	.beginModule("qt")
		.beginExtendClass<QLayout, QWidget>("QLayout")
			.addFunction("addWidget", &QLayout::addWidget)
			.addFunction("count", &QLayout::count)
		.endClass()

		.beginExtendClass<QBoxLayout, QLayout>("QBoxLayout")
		.endClass()

		.beginExtendClass<QHBoxLayout, QBoxLayout>("QHBoxLayout")
			.addConstructor(LUA_ARGS())
		.endClass()

		.beginExtendClass<QVBoxLayout, QBoxLayout>("QVBoxLayout")
			.addConstructor(LUA_ARGS())
		.endClass()

		.beginExtendClass<QGroupBox, QWidget>("QGroupBox")
		.endClass()
	.endModule();
}

void addQtWidgetsBindings(lua_State *L) {
	LuaIntf::LuaBinding(L)

	.beginModule("qt")
		.beginExtendClass<QAbstractButton, QWidget>("QAbstractButton")
			.addFunction("setIcon", &QAbstractButton::setIcon)
			.addFunction("setIconSize", &QAbstractButton::setIconSize)
			.addFunction("setCheckable", &QAbstractButton::setCheckable)
		.endClass()

        .beginExtendClass<QComboBox, QWidget>("QComboBox")
        .endClass()

		.beginExtendClass<QPushButton, QAbstractButton>("QPushButton")
			.addFactory([](QString name) {
				return new QPushButton(name);
			})
			.addFunction("setFlat", &QPushButton::setFlat)
			.addFunction("setMenu", &QPushButton::setMenu)
		.endClass()
	.endModule();
}

void addLCBindings(lua_State *L) {
	LuaIntf::LuaBinding(L)

	.beginModule("lc")
		.beginExtendClass<CadMdiChild, QWidget>("CadMdiChild")
			.addFactory([]() {
				return new CadMdiChild;
			})
			.addFunction("getSnapManager", &CadMdiChild::getSnapManager)
			.addFunction("cursor", &CadMdiChild::cursor)
			.addFunction("document", &CadMdiChild::document)
            .addFunction("saveFile", &CadMdiChild::saveFile)
			.addProperty("id", &CadMdiChild::id, &CadMdiChild::setId)
			.addFunction("openFile", &CadMdiChild::openFile)
			.addFunction("selection", &CadMdiChild::selection)
			.addFunction("newDocument", &CadMdiChild::newDocument)
			.addFunction("setDestroyCallback", &CadMdiChild::setDestroyCallback)
			.addFunction("tempEntities", &CadMdiChild::tempEntities)
			.addFunction("undoManager", &CadMdiChild::undoManager)
			.addFunction("viewer", &CadMdiChild::viewer)
			.addFunction("activeLayer", &CadMdiChild::activeLayer)
			.addFunction("metaInfoManager", &CadMdiChild::metaInfoManager)
		.endClass()

		.beginClass<drawable::Cursor>("Cursor")
			.addFunction("position", &drawable::Cursor::position)
		.endClass()

		.beginExtendClass<LCADViewer, QWidget>("LCADViewer")
			.addFunction("x", &LCADViewer::x)
			.addFunction("y", &LCADViewer::y)
			.addFunction("autoScale", &LCADViewer::autoScale)
			.addFunction("setOperationActive", &LCADViewer::setOperationActive)
			.addFunction("docCanvas", &LCADViewer::docCanvas)
		.endClass()
		
		.beginClass<LuaInterface>("LuaInterface")
			.addFunction("luaConnect", &LuaInterface::luaConnect)
			.addFunction("connect", &LuaInterface::qtConnect)
			.addFunction("pluginList", &LuaInterface::pluginList)
			.addFunction("operation", &LuaInterface::operation)
			.addFunction("setOperation", &LuaInterface::setOperation)
			.addFunction("registerEvent", &LuaInterface::registerEvent)
			.addFunction("deleteEvent", &LuaInterface::deleteEvent)
			.addFunction("triggerEvent", &LuaInterface::triggerEvent)
		.endClass()
		
		.beginExtendClass<widgets::LuaScript, QDockWidget>("LuaScript")
			.addConstructor(LUA_ARGS(QMdiArea*, widgets::CliCommand*))
		.endClass()

		.beginClass<DocumentCanvas>("DocumentCanvas")
			.addFunction("autoScale", &DocumentCanvas::autoScale)
			.addFunction("selectPoint", &DocumentCanvas::selectPoint)
		.endClass()

		.beginExtendClass<widgets::CliCommand, QDockWidget>("CliCommand")
			.addConstructor(LUA_SP(std::shared_ptr<widgets::CliCommand>), LUA_ARGS(LuaIntf::_opt<QWidget*>))
			.addFunction("addCommand", &widgets::CliCommand::addCommand)
			.addFunction("write", &widgets::CliCommand::write, LUA_ARGS(const char*))
			.addFunction("returnText", &widgets::CliCommand::returnText)
		.endClass()

		.beginExtendClass<widgets::Toolbar, QDockWidget>("Toolbar")
			.addConstructor(LUA_SP(std::shared_ptr<widgets::Toolbar>), LUA_ARGS(LuaIntf::_opt<QWidget*>))
			.addFunction("addTab", &widgets::Toolbar::addTab)
			.addFunction("removeTab", &widgets::Toolbar::removeTab)
			.addFunction("tabByName", &widgets::Toolbar::tabByName)
		.endClass()

		.beginExtendClass<widgets::ToolbarTab, QDockWidget>("ToolbarTab")
			.addFactory([]() {
				return new widgets::ToolbarTab();
			})
			.addFunction("addButton", &widgets::ToolbarTab::addButton, LUA_ARGS(QGroupBox*,
																			 const char*,
																			 LuaIntf::_opt<int>,
																			 LuaIntf::_opt<int>,
																			 LuaIntf::_opt<int>,
																			 LuaIntf::_opt<int>))
            .addFunction("addWidget", &widgets::ToolbarTab::addWidget, LUA_ARGS(QGroupBox*,
                                                                         QWidget*,
                                                                         LuaIntf::_opt<int>,
                                                                         LuaIntf::_opt<int>,
                                                                         LuaIntf::_opt<int>,
                                                                         LuaIntf::_opt<int>))
			.addFunction("addGroup", &widgets::ToolbarTab::addGroup)
			.addFunction("buttonByText", &widgets::ToolbarTab::buttonByText)
			.addFunction("groupByName", &widgets::ToolbarTab::groupByName)
			.addFunction("removeGroup", &widgets::ToolbarTab::removeGroup)
		.endClass()

		.beginClass<drawable::TempEntities>("TempEntities")
			.addFunction("addEntity", &drawable::TempEntities::addEntity)
			.addFunction("removeEntity", &drawable::TempEntities::removeEntity)
		.endClass()

        .beginClass<lc::ui::MetaInfoManager>("MetaInfoManager")
			.addFunction("metaInfo", &lc::ui::MetaInfoManager::metaInfo)
        .endClass()

		.beginExtendClass<widgets::Layers, QDockWidget>("Layers")
            .addFactory([]() {
                return new widgets::Layers();
            })
			.addFunction("setMdiChild", &widgets::Layers::setMdiChild, LUA_ARGS(LuaIntf::_opt<CadMdiChild*>))
		.endClass()

		.beginClass<manager::SnapManagerImpl>("SnapManager")
			.addFunction("setGridSnappable", &manager::SnapManagerImpl::setGridSnappable)
		.endClass()

		.beginExtendClass<dialog::LinePatternManager, QDialog>("LinePatternManager")
            .addFactory([](lc::storage::Document_SPtr document){
                return new dialog::LinePatternManager(document);
            })
            .addFunction("setDocument", &dialog::LinePatternManager::setDocument)
        .endClass()

        .beginExtendClass<lc::ui::widgets::LinePatternSelect, QComboBox>("LinePatternSelect")
            .addFactory([](QWidget* parent, bool showByLayer, bool showByBlock){
                return new lc::ui::widgets::LinePatternSelect(nullptr, parent, showByLayer, showByBlock);
            })
            .addFunction("setMdiChild", &lc::ui::widgets::LinePatternSelect::setMdiChild, LUA_ARGS(LuaIntf::_opt<CadMdiChild*>))
        .endClass()

        .beginExtendClass<lc::ui::widgets::LineWidthSelect, QComboBox>("LineWidthSelect")
            .addFactory([](QWidget* parent, bool showByLayer, bool showByBlock){
                return new lc::ui::widgets::LineWidthSelect(nullptr, parent, showByLayer, showByBlock);
            })
            .addFunction("setMetaInfoManager",
                         &lc::ui::widgets::LineWidthSelect::setMetaInfoManager,
                         LUA_ARGS(LuaIntf::_opt<lc::ui::MetaInfoManager_SPtr>)
            )
        .endClass()

        .beginExtendClass<lc::ui::widgets::ColorSelect, QComboBox>("ColorSelect")
            .addFactory([](QWidget* parent, bool showByLayer, bool showByBlock){
                return new lc::ui::widgets::ColorSelect(nullptr, parent, showByLayer, showByBlock);
            })
            .addFunction("setMetaInfoManager",
                         &lc::ui::widgets::ColorSelect::setMetaInfoManager,
                         LUA_ARGS(LuaIntf::_opt<lc::ui::MetaInfoManager_SPtr>)
            )
        .endClass()

	.endModule();
}


void addQtMetaTypes() {
	qRegisterMetaType<lc::geo::Coordinate>();
}
