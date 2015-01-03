#pragma once

#include <memory>

#include <cad/operations/builder.h>
#include <cad/document/document.h>
#include <cad/meta/layer.h>

#include <cad/operations/layerops.h>
#include "lualibrecadbridge.h"

#include "lua-intf/LuaIntf/LuaIntf.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/primitive/line.h"
#include "cad/primitive/coordinate.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/text.h"
#include "cad/meta/color.h"
#include "cad/base/metainfo.h"
#include "cad/operations/builder.h"
#include "cad/operations/builderops.h"
#include "cad/dochelpers/documentimpl.h"
#include "cad/operations/documentoperation.h"
#include "cad/dochelpers/storagemanagerimpl.h"


struct lua_State;
void lua_openlckernel(lua_State* L);
