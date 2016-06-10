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
			.addConstructor(LUA_ARGS(QObject*))
			.addFunction("setText", &QAction::setText)
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
			.addConstructor(LUA_ARGS(QWidget*))
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
	.endModule();
}

void addQtWidgetsBindings(lua_State *L) {
	LuaIntf::LuaBinding(L)

	.beginModule("qt")
		.beginExtendClass<QAbstractButton, QWidget>("QAbstractButton")
		.endClass()

		.beginExtendClass<QPushButton, QAbstractButton>("QPushButton")
			.addConstructor(LUA_ARGS(QString))
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
			.addConstructor(LUA_ARGS(QMdiArea*))
		.endClass()

		.beginClass<LCViewer::DocumentCanvas>("DocumentCanvas")
			.addFunction("autoScale", &LCViewer::DocumentCanvas::autoScale)
		.endClass()

		.beginExtendClass<CliCommand, QDockWidget>("CliCommand")
			.addConstructor(LUA_SP(std::shared_ptr<CliCommand>), LUA_ARGS(QWidget*))
			.addFunction("addCommand", &CliCommand::addCommand)
			.addFunction("write", &CliCommand::write, LUA_ARGS(const char*))
		.endClass()

	.endModule();
}

void addQtMetaTypes() {
	qRegisterMetaType<lc::geo::Coordinate>();
}
