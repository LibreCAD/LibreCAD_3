#pragma once

#include <QApplication>
#include <luainterface.h>
#include <cad/dochelpers/documentimpl.h>

static int argc = 0;
static char** argv = NULL;

LuaIntf::LuaState startLC();
lc::Document_SPtr createDocument();