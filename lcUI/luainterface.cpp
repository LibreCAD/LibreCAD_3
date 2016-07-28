#include "luainterface.h"

LuaInterface::LuaInterface() {
	_L = LuaIntf::LuaState::newState();
}

LuaInterface::~LuaInterface() {
	_luaQObjects.clear();

	_L.close();
}

void LuaInterface::initLua() {
    luaL_openlibs(_L);
    luaOpenQtBridge(_L);
    lua_openlckernel(_L);

    LuaIntf::Lua::setGlobal(_L, "luaInterface", this);

    QString luaFile = QCoreApplication::applicationDirPath() + "/path.lua";
    int s = _L.doFile(luaFile.toStdString().c_str());

    if (s != 0) {
        std::cout << lua_tostring(_L, -1) << std::endl;
        lua_pop(_L, 1);
    }
}

bool LuaInterface::luaConnect(
	QObject* sender,
	std::string signalName,
	LuaIntf::LuaRef slot)
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

bool LuaInterface::qtConnect(QObject *sender, std::string signalName, QObject *receiver, std::string slotName) {
	int signalId = sender->metaObject()->indexOfSignal(signalName.c_str());
	if(signalId < 0) {
		std::cout << "No such signal " << signalName << std::endl;
	}

	int slotId = receiver->metaObject()->indexOfSlot(slotName.c_str());
	if(slotId < 0) {
		std::cout << "No such slot " << signalName << std::endl;
	}

	return QMetaObject::connect(sender, signalId, receiver, slotId);
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
    for(auto dir : list) {
        plugins.push_back(dir.toStdString());
    }

    return plugins;
}