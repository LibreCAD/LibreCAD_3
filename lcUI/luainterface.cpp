#include <lclua.h>
#include <managers/luacustomentitymanager.h>
#include "luainterface.h"

#include <QDir>
#include <QFileDialog>

#include "mainwindow.h"
#include "operationloader.h"

using namespace lc::ui;

LuaInterface::LuaInterface() :
	_pluginManager(_L.state(), "gui"){
}

LuaInterface::~LuaInterface() {
    _events.clear();

    lc::lua::LuaCustomEntityManager::getInstance().removePlugins();
}

void LuaInterface::initLua(QMainWindow* mainWindow) {
	auto lcLua = lc::lua::LCLua(_L.state());
    lcLua.setF_openFileDialog(&LuaInterface::openFileDialog);
    lcLua.addLuaLibs();
	lcLua.importLCKernel();

    luaOpenGUIBridge(_L.state());

    _L["luaInterface"] = this;
    registerGlobalFunctions(mainWindow);

    QString luaFile = QCoreApplication::applicationDirPath() + "/path.lua";
    bool s = _L.dofile(luaFile.toStdString().c_str());

    std::string luaPath = _L["lua_path"];
    lc::ui::OperationLoader opLoader(luaPath, mainWindow, _L);

    if (s) {
		const char* out = lua_tostring(_L.state(), -1);
		if (out == nullptr) {
			LOG_WARNING << "Lua output null" << std::endl;
		}
		else {
			LOG_INFO << "Lua output:" << out << std::endl;
		}
        lua_pop(_L.state(), 1);
    }

	_pluginManager.loadPlugins();
}

QWidget* LuaInterface::loadUiFile(const char* fileName) {
	QUiLoader uiLoader;
	QFile file(fileName);
    file.open(QFile::ReadOnly);

    QWidget* widget = uiLoader.load(&file);

    file.close();

    return widget;
}

void LuaInterface::hideUI(bool hidden) {
	_L["hideUI"] = hidden;
}

lua_State* LuaInterface::luaState() {
	return _L.state();
}

std::vector<std::string> LuaInterface::pluginList(const char* path) {
    std::vector<std::string> plugins;
    QDir dir(path);

    auto list = dir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    for(const auto& dir : list) {
        plugins.push_back(dir.toStdString());
    }

    return plugins;
}

FILE* LuaInterface::openFileDialog(bool isOpening, const char* description, const char* mode) {
    QString path;

    if(isOpening) {
        path = QFileDialog::getOpenFileName(nullptr, (std::string("Open ") + description).c_str());
    }
    else {
        path = QFileDialog::getSaveFileName(nullptr, (std::string("Save ") + description).c_str());
    }

    if(path.isEmpty()) {
        return nullptr;
    }

    return fopen(path.toStdString().c_str(), mode);
}

kaguya::LuaRef LuaInterface::operation() {
    return _operation;
}

void LuaInterface::setOperation(kaguya::LuaRef operation) {
    _operation = std::move(operation);
}

void LuaInterface::finishOperation() {
    if(!_operation.isNilref() && !_operation["close"].isNilref()){
        _operation["close"](_operation);
    }
}

void LuaInterface::registerEvent(const std::string& event, const kaguya::LuaRef& callback) {
    if(callback.type() == LUA_TTABLE && callback["onEvent"].isNilref()) {
        return;
    }

    _events[event].push_back(callback);
}

void LuaInterface::deleteEvent(const std::string& event, const kaguya::LuaRef& callback) {
    auto it = std::find(_events[event].begin(), _events[event].end(), callback);

    if(it != _events[event].end()) {
        _events[event].erase(it);
    }
}

void LuaInterface::triggerEvent(const std::string& event, kaguya::LuaRef args) {
    auto events = _events[event];
    for(auto eventCallback : events) {
        if(eventCallback.type() == LUA_TFUNCTION) {
            eventCallback(event, args);
        }
        else if(eventCallback.type() == LUA_TTABLE) {
            eventCallback["onEvent"](eventCallback, event, args);
        }
    }
}

void LuaInterface::registerGlobalFunctions(QMainWindow* mainWindow) {
    // register common functions i.e. run_basic_operation and message
    _L["mainWindow"] = static_cast<lc::ui::MainWindow*>(mainWindow);
    _L.dostring("run_basic_operation = function(operation, init_method) mainWindow:runOperation(operation, init_method) end");
    _L.dostring("finish_operation = function() luaInterface:finishOperation() end");
    _L.dostring("operationFinished = function() mainWindow:operationFinished() end");

    // cli command helper functions
    _L.dostring("message = function(m) mainWindow:cliCommand():write(tostring(m)) end");
    _L.dostring("cli_get_text = function(getText) mainWindow:cliCommand():returnText(getText) end");
    _L.dostring("add_command = function(command, callback) mainWindow:cliCommand():addCommand(command, callback) end");
    _L.dostring("run_command = function(command) mainWindow:cliCommand():runCommand(command) end");
    _L.dostring("add_command('CLEAR', function() mainWindow:cliCommand():clear() end)");
    _L.dostring("CreateDialogWidget = function(widgetName) return gui.DialogWidget(widgetName,mainWindow) end");

    _L.dostring("luaInterface:registerEvent('finishOperation', finish_operation)");
    _L.dostring("luaInterface:registerEvent('operationFinished', operationFinished)");
}
