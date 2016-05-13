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
		LuaQObject(QObject *object);
		~LuaQObject();
		
		bool connect(int signalId, LuaIntf::LuaRef slot);
		
		std::string objectName();
		static std::string objectName(QObject* object);
		
		QObject* findChild(std::string name);
		static QObject* findChild(QObject* object, std::string name);
		

		int qt_metacall(QMetaObject::Call c, int id, void **a);

	private:
	
		QObject* _object;

		
		
		int _slotId;
		LuaIntf::LuaRef _slotFunction;
};

using LuaQObject_SPtr = std::shared_ptr<LuaQObject>;
using LuaQObject_CSPtr = std::shared_ptr<const LuaQObject>;