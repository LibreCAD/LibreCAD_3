#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <iostream>

#include <QMetaObject>
#include <QMetaMethod>
#include <QObject>
#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QtUiTools/QUiLoader>
#include <QCoreApplication>
#include "lua/luaqobject.h"

#include "lua-intf/LuaIntf/LuaIntf.h"
#include "cad/lualibrecadbridge.h"
#include "lua/qtbridge.h"

class LuaInterface {
	public:
		LuaInterface();
		~LuaInterface();

		void initLua();
			
		int qtConnect(
			lua_State *L,
			QObject *sender,
			std::string signalName,
			std::string luaFunction
		);

		static std::shared_ptr<QWidget> loadUiFile(const char* fileName);

	private:		
		lua_State* _L;
		std::vector<LuaQObject_SPtr> _luaQObjects;
};