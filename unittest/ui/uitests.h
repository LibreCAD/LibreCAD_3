#pragma once

#include <luainterface.h>
#include <cad/storage/documentimpl.h>
#include <QApplication>
#include <cad/storage/storagemanagerimpl.h>
#include <cadmdichild.h>

static int argc = 1;
static char* argv[2] = {strdup("lcunittest"), NULL};

using namespace lc;
using namespace ui;

storage::Document_SPtr createDocument();
