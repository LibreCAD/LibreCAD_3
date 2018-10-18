#include <lclua.h>
#include <managers/luacustomentitymanager.h>
#include "luainterface.h"

using namespace lc::ui;

LuaInterface::LuaInterface() :
    _L(LuaIntf::LuaState::newState()),
	_pluginManager(_L, "gui") {
}

LuaInterface::~LuaInterface() {
	_luaQObjects.clear();
    _operations.clear();
    _events.clear();

    lc::lua::LuaCustomEntityManager::getInstance().removePlugins();

	_L.close();
}

void LuaInterface::initLua() {
	auto lcLua = lc::lua::LCLua(_L);
    lcLua.setF_openFileDialog(&LuaInterface::openFileDialog);
    lcLua.addLuaLibs();
	lcLua.importLCKernel();

    luaOpenQtBridge(_L);

    LuaIntf::Lua::setGlobal(_L, "luaInterface", this);

    QString luaFile = QCoreApplication::applicationDirPath() + "/path.lua";
    bool s = _L.doFile(luaFile.toStdString().c_str());

    if (s) {
        std::cout << lua_tostring(_L, -1) << std::endl;
        lua_pop(_L, 1);
    }

	_pluginManager.loadPlugins();
}

bool LuaInterface::luaConnect(
	QObject* sender,
	const std::string& signalName,
	const LuaIntf::LuaRef& slot)
{
	int signalId = sender->metaObject()->indexOfSignal(signalName.c_str());
	
	if(signalId < 0) {
		std::cout << "No such signal " << signalName << std::endl;
	}
	else {
		auto lqo = std::make_shared<LuaQObject>(sender);
		_luaQObjects.push_back(lqo);

		auto connected = lqo->connect(signalId, slot);

		cleanInvalidQObject();

		return connected;
	}

	return false;
}

std::shared_ptr<QWidget> LuaInterface::loadUiFile(const char* fileName) {
	QUiLoader uiLoader;
	QFile file(fileName);
    file.open(QFile::ReadOnly);

    std::shared_ptr<QWidget> widget(uiLoader.load(&file));

    file.close();

    return widget;
}

void LuaInterface::cleanInvalidQObject() {
	_luaQObjects.erase(std::remove_if(_luaQObjects.begin(),
									  _luaQObjects.end(),
							  [](LuaQObject_SPtr lqo){
								  return !lqo->valid();
							  }),
					   _luaQObjects.end());
}

bool LuaInterface::qtConnect(QObject *sender, const std::string& signalName, QObject *receiver, const std::string& slotName) {
	int signalId = sender->metaObject()->indexOfSignal(signalName.c_str());
	if(signalId < 0) {
		std::cout << "No such signal " << signalName << std::endl;
	}

	int slotId = receiver->metaObject()->indexOfSlot(slotName.c_str());
	if(slotId < 0) {
		std::cout << "No such slot " << slotName << std::endl;
	}

	return QMetaObject::connect(sender, signalId, receiver, slotId) != nullptr;
}

void LuaInterface::hideUI(bool hidden) {
	LuaIntf::Lua::setGlobal(_L, "hideUI", hidden);
}

LuaIntf::LuaState LuaInterface::luaState() {
	return _L;
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

LuaIntf::LuaRef LuaInterface::operation(unsigned int windowID) {
    if(_operations.find(windowID) != _operations.end()) {
        return _operations[windowID];
    }

    return LuaIntf::LuaRef();
}

void LuaInterface::setOperation(unsigned int windowID, LuaIntf::LuaRef operation) {
    _operations[windowID] = std::move(operation);
}

void LuaInterface::registerEvent(const std::string& event, const LuaIntf::LuaRef& callback) {
    if(callback.isTable() && !callback.has("onEvent")) {
        return;
    }

    _events[event].push_back(callback);
}

void LuaInterface::deleteEvent(const std::string& event, const LuaIntf::LuaRef& callback) {
    auto it = std::find(_events[event].begin(), _events[event].end(), callback);

    if(it != _events[event].end()) {
        _events[event].erase(it);
    }
}

void LuaInterface::triggerEvent(const std::string& event, LuaIntf::LuaRef args) {
    auto events = _events[event];
    for(auto eventCallback : events) {
        if(eventCallback.isFunction()) {
            eventCallback(event, args);
        }
        else if(eventCallback.isTable()) {
            eventCallback.get("onEvent").call(eventCallback, event, args);
        }
    }
}
