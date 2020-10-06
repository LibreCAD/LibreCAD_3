#pragma once
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <kaguya/kaguya.hpp>


void luaOpenGUIBridge(lua_State *L);

void addLCBindings(lua_State *L);
void addLuaGUIAPIBindings(lua_State* L);
