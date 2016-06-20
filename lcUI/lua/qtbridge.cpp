#include "lua/qtbridge.h"
#include "luaqobject.h"
#include "luainterface.h"

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
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
			.addFunction("cursor", &CadMdiChild::cursor)
			.addFunction("document", &CadMdiChild::document)
			.addProperty("id", &CadMdiChild::id, &CadMdiChild::setId)
			.addFunction("import", &CadMdiChild::import)
			.addFunction("newDocument", &CadMdiChild::newDocument)
			.addFunction("setDestroyCallback", &CadMdiChild::setDestroyCallback)
			.addFunction("tempEntities", &CadMdiChild::tempEntities)
			.addFunction("undoManager", &CadMdiChild::undoManager)
			.addFunction("view", &CadMdiChild::view)
			.addFunction("viewer", &CadMdiChild::viewer)
		.endClass()

		.beginClass<LCViewer::Cursor>("Cursor")
			.addFunction("position", &LCViewer::Cursor::position)
		.endClass()

		.beginExtendClass<LCViewer::LCADViewer, QWidget>("LCADViewer")
			.addFunction("x", &LCViewer::LCADViewer::x)
			.addFunction("y", &LCViewer::LCADViewer::y)
			.addFunction("autoScale", &LCViewer::LCADViewer::autoScale)
		.endClass()
		
		.beginClass<LuaInterface>("LuaInterface")
			.addFunction("luaConnect", &LuaInterface::luaConnect)
			.addFunction("connect", &LuaInterface::qtConnect)
		.endClass()
		
		.beginExtendClass<LuaScript, QDockWidget>("LuaScript")
			.addConstructor(LUA_ARGS(QMdiArea*, CliCommand*))
		.endClass()

		.beginClass<LCViewer::DocumentCanvas>("DocumentCanvas")
			.addFunction("autoScale", &LCViewer::DocumentCanvas::autoScale)
		.endClass()

		.beginExtendClass<CliCommand, QDockWidget>("CliCommand")
			.addConstructor(LUA_SP(std::shared_ptr<CliCommand>), LUA_ARGS(LuaIntf::_opt<QWidget*>))
			.addFunction("addCommand", &CliCommand::addCommand)
			.addFunction("write", &CliCommand::write, LUA_ARGS(const char*))
			.addFunction("returnText", &CliCommand::returnText)
		.endClass()

		.beginExtendClass<Toolbar, QDockWidget>("Toolbar")
			.addConstructor(LUA_SP(std::shared_ptr<Toolbar>), LUA_ARGS(LuaIntf::_opt<QWidget*>))
			.addFunction("addTab", &Toolbar::addTab)
			.addFunction("removeTab", &Toolbar::removeTab)
			.addFunction("tabByName", &Toolbar::tabByName)
		.endClass()

		.beginExtendClass<ToolbarTab, QDockWidget>("ToolbarTab")
			.addFactory([]() {
				return new ToolbarTab();
			})
			.addFunction("addButton", &ToolbarTab::addButton, LUA_ARGS(QGroupBox*,
																	   QPushButton*,
																	   LuaIntf::_opt<int>,
																	   LuaIntf::_opt<int>,
																	   LuaIntf::_opt<int>,
																	   LuaIntf::_opt<int>))
			.addFunction("addButtonStr", &ToolbarTab::addButtonStr, LUA_ARGS(QGroupBox*,
																			 const char*,
																			 LuaIntf::_opt<int>,
																			 LuaIntf::_opt<int>,
																			 LuaIntf::_opt<int>,
																			 LuaIntf::_opt<int>))
			.addFunction("addGroup", &ToolbarTab::addGroup)
			.addFunction("buttonByText", &ToolbarTab::buttonByText)
			.addFunction("groupByName", &ToolbarTab::groupByName)
		.endClass()

		.beginClass<LCViewer::TempEntities>("TempEntities")
			.addFunction("addEntity", &LCViewer::TempEntities::addEntity)
			.addFunction("removeEntity", &LCViewer::TempEntities::removeEntity)
		.endClass()
	.endModule();
}

void addQtMetaTypes() {
	qRegisterMetaType<lc::geo::Coordinate>();
}
