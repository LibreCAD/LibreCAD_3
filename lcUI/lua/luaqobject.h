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
		LuaQObject(QObject* object);
		~LuaQObject();
		
		bool connect(int signalId, LuaIntf::LuaRef slot);
		void pushArg(LuaIntf::LuaState s, int const type, void const* arg);
		
		std::string objectName();
		static std::string objectName(QObject* object);

		QObject* findChild(std::string name);
		static QObject* findChild(QObject* object, std::string name);

		bool valid();

		int qt_metacall(QMetaObject::Call c, int id, void **a);

	private:
		QObject* _object;

		int _slotId;
		int _signalId;
		LuaIntf::LuaRef _slotFunction;

		bool _valid;
};

using LuaQObject_SPtr = std::shared_ptr<LuaQObject>;
using LuaQObject_CSPtr = std::shared_ptr<const LuaQObject>;