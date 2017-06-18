#pragma once

#include <luainterface.h>
#include <cad/dochelpers/documentimpl.h>
#include <QApplication>
#include <cad/dochelpers/storagemanagerimpl.h>

static int argc = 0;
static char** argv = NULL;

LuaInterface* startLC();
lc::Document_SPtr createDocument();
CadMdiChild* getMdiChild(lua_State* L);