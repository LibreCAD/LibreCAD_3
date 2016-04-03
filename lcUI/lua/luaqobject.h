#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <QObject>
#include <QMetaMethod>

#include "lua-intf/LuaIntf/LuaIntf.h"

class LuaQObject : public QObject {
	public:
		LuaQObject(QObject *object, lua_State* L);
		int connect(int signalId, std::string luaFunction);
		
		std::string objectName();
		static std::string objectName(QObject* object);
		
		QObject* findChild(std::string name);
		static QObject* findChild(QObject* object, std::string name);
		

		int qt_metacall(QMetaObject::Call c, int id, void **a);

	private:
	
		QObject* _object;
		lua_State* _L;
		
		int _slotId;
		LuaIntf::LuaRef _slotFunction;
};