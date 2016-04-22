#pragma once

#include <memory>

#include "lualibrecadbridge.h"
#include "lua-intf/LuaIntf/LuaIntf.h"

#include <cad/operations/layerops.h>
#include <cad/operations/builder.h>
#include <cad/document/document.h>
#include <cad/document/undomanager.h>
#include <cad/meta/layer.h>
#include <cad/meta/metacolor.h>
#include <cad/geometry/geocoordinate.h>
#include <cad/primitive/line.h>
#include <cad/primitive/point.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/text.h>
#include <cad/primitive/dimradial.h>
#include <cad/primitive/dimdiametric.h>
#include <cad/primitive/dimlinear.h>
#include <cad/primitive/dimaligned.h>
#include <cad/primitive/dimangular.h>
#include <cad/primitive/spline.h>
#include <cad/primitive/lwpolyline.h>
#include <cad/meta/color.h>
#include <cad/base/metainfo.h>
#include <cad/operations/builder.h>
#include <cad/operations/builderops.h>
#include <cad/dochelpers/documentimpl.h>
#include <cad/dochelpers/undomanagerimpl.h>
#include <cad/operations/documentoperation.h>
#include <cad/dochelpers/storagemanagerimpl.h>
#include <cad/interface/snapable.h>
#include <cad/interface/metatype.h>


struct lua_State;
void lua_openlckernel(lua_State* L);
