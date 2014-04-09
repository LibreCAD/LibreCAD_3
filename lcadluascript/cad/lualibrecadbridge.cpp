extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "lualibrecadbridge.h"
#include "lua-intf/LuaIntf.h"

#include <cad/geometry/geocoordinate.h>
#include <cad/primitive/line.h>
#include <cad/primitive/circle.h>
#include <cad/document/abstractdocument.h>
#include <cad/dochelpers/documentimpl.h>
#include <cad/operations/createentities.h>

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(boost::shared_ptr)
}

using namespace LuaIntf;


boost::shared_ptr<LuaCreateEntities > lua_CreateEntities(lc::AbstractDocument * doc) {
    return boost::make_shared< LuaCreateEntities>(doc, "0");
}

void lua_openlckernel(lua_State * L)
{

    LuaBinding(L)
            .beginClass <lc::Operation> ("Operation")
            .endClass ()

            .beginClass <lc::CADEntity> ("CADEntity")
            .endClass ()

            .beginClass <lc::geo::Vector> ("Vector")
            .addConstructor(LUA_SP(shared_ptr<const lc::geo::Vector>), LUA_ARGS(const lc::geo::Coordinate& start, const lc::geo::Coordinate& end))
            .endClass ()

            .beginClass <lc::AbstractDocument> ("AbstractDocument")
            .addFunction ("operateOn", &lc::AbstractDocument::operateOn)
            .endClass ()

            .beginClass<lc::geo::Coordinate>("Coord")
            .addConstructor(LUA_ARGS(double x, double y))
            .addFunction("x", &lc::geo::Coordinate::x)
            .addFunction("y", &lc::geo::Coordinate::y)
            .addFunction("z", &lc::geo::Coordinate::z)
            .endClass()

            .beginClass<lc::geo::Coordinate>("Coordinate")
            .addConstructor(LUA_ARGS(double x, double y))
            .addFunction("x", &lc::geo::Coordinate::x)
            .addFunction("y", &lc::geo::Coordinate::y)
            .addFunction("z", &lc::geo::Coordinate::z)
            .endClass()

            .beginExtendClass <lc::Line, lc::CADEntity> ("Line")
            .addConstructor(LUA_SP(shared_ptr< lc::Line>), LUA_ARGS(const lc::geo::Coordinate& start, const lc::geo::Coordinate& end))
            .endClass()

            .beginExtendClass <lc::Circle, lc::CADEntity> ("Circle")
            .addConstructor(LUA_SP(shared_ptr< lc::Circle>), LUA_ARGS(const lc::geo::Coordinate& center, double radius))
            .endClass()

            .beginExtendClass <LuaCreateEntities, lc::Operation> ("CreateEntities")
            .addConstructor(LUA_SP(shared_ptr<LuaCreateEntities>), LUA_ARGS(lc::AbstractDocument * doc,  const char * layerName))
            .addFunction ("append", &LuaCreateEntities::append)
            .endClass()

            .beginClass <lc::DocumentImpl> ("DocumentImpl")
            .addFunction ("operateOn", &lc::DocumentImpl::operateOn)
            .endClass ()


            .beginClass <QString> ("QString")
            .addConstructor(LUA_ARGS(const char *))
            .endClass ();

}

// .addConstructor(LUA_SP(shared_ptr<const lc::CreateEntities>), LUA_ARGS(lc::AbstractDocument * doc))


