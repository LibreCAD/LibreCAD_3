#include "luainterface.h"

LuaInterface::LuaInterface() {
	_L = luaL_newstate();
	
	luaL_openlibs(_L);
	luaOpenQtBridge(_L);
	lua_openlckernel(_L);

	LuaIntf::Lua::setGlobal(_L, "luaInterface", this);
}

LuaInterface::~LuaInterface() {
	_luaQObjects.clear();
	
	lua_close(_L);
}

void LuaInterface::initLua() {
	std::string out;
	QString luaFile = QCoreApplication::applicationDirPath() + "/path.lua";
	int s = luaL_dofile(_L, luaFile.toStdString().c_str());

	if (s != 0) {
        out.append(lua_tostring(_L, -1));
        lua_pop(_L, 1);
    }
    
    std::cout << out << std::endl;
}

int LuaInterface::qtConnect(
	lua_State* L,
	QObject *sender,
	std::string signalName,
	std::string luaFunction)
{
	int signalId = sender->metaObject()->indexOfSignal(signalName.c_str());
	
	if(signalId < 0) {
		std::cout << "No such signal " << signalName << std::endl;
	}
	else {
		LuaQObject_SPtr lqo(new LuaQObject(sender, L));
		_luaQObjects.push_back(lqo);
		return lqo->connect(signalId, luaFunction);
	}

	return 0;
}

std::shared_ptr<QWidget> LuaInterface::loadUiFile(const char* fileName) {
	QUiLoader uiLoader;
	QFile file(fileName);
    file.open(QFile::ReadOnly);

    std::shared_ptr<QWidget> widget(uiLoader.load(&file));

    file.close();

    return widget;
}