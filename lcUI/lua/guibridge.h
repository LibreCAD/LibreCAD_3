#pragma once
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <QObject>
#include <QMetaObject>

#include "cad/geometry/geocoordinate.h"
#include <kaguya/kaguya.hpp>


Q_DECLARE_METATYPE(lc::geo::Coordinate);

void luaOpenGUIBridge(lua_State *L);

void addLCBindings(lua_State *L);
void addLuaGUIAPIBindings(lua_State* L);
void addQtMetaTypes();
