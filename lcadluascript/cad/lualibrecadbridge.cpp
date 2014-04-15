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
#include "LuaIntf.h"

#include <cad/geometry/geocoordinate.h>
#include <cad/primitive/line.h>
#include <cad/primitive/circle.h>
#include <cad/document/abstractdocument.h>
#include <cad/dochelpers/documentimpl.h>
#include <cad/operations/builder.h>
#include <cad/operations/builder.h>
#include <cad/operations/create.h>

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(boost::shared_ptr)
}

using namespace LuaIntf;

void lua_openlckernel(lua_State * L)
{

    LuaBinding(L)
            .beginClass <lc::operation::Operation> ("Operation")
            .addFunction("execute", &lc::operation::Operation::execute)
            .endClass ()

            .beginClass <lc::ID> ("ID")
            .addFunction("id", &lc::ID::id)
            .endClass ()

            .beginExtendClass <lc::CADEntity, lc::ID> ("CADEntity")
            .endClass ()

            .beginClass <lc::geo::Vector> ("Vector")
            .addConstructor(LUA_SP(shared_ptr<const lc::geo::Vector>), LUA_ARGS(const lc::geo::Coordinate& start, const lc::geo::Coordinate& end))
            .endClass ()

            .beginClass <lc::AbstractDocument> ("AbstractDocument")
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

            .beginExtendClass <LuaCreateEntities, lc::operation::Operation> ("Create")
            .addConstructor(LUA_SP(shared_ptr<LuaCreateEntities>), LUA_ARGS(lc::AbstractDocument * doc,  const char * layerName))
            .addFunction ("append", &LuaCreateEntities::append)
            .endClass()

            .beginClass <lc::DocumentImpl> ("DocumentImpl")
            .endClass ()

            .beginClass <QString> ("QString")
            .addConstructor(LUA_ARGS(const char *))
            .endClass ()

            .beginExtendClass <LuaBuilder,lc::operation::Operation> ("Builder")
            .addConstructor(LUA_SP(shared_ptr<LuaBuilder>), LUA_ARGS(lc::AbstractDocument * doc))
            .addFunction ("append", &LuaBuilder::append)
            .addFunction ("move", &LuaBuilder::move)
            .addFunction ("copy", &LuaBuilder::copy)
            .addFunction ("foo", &LuaBuilder::repeat)
            .endClass ()

            .beginClass <lc::operation::BBase> ("BBase")
            .endClass ()
            .beginExtendClass <lc::operation::BMove, lc::operation::BBase> ("BMove")
            .addConstructor(LUA_SP(shared_ptr< lc::operation::BMove>), LUA_ARGS(const lc::geo::Coordinate& offset))
            .endClass()
            .beginExtendClass <lc::operation::BCopy, lc::operation::BBase> ("BCopy")
            .addConstructor(LUA_SP(shared_ptr< lc::operation::BCopy>), LUA_ARGS(const lc::geo::Coordinate& offset))
            .endClass();


}

// .addConstructor(LUA_SP(shared_ptr<const lc::CreateEntities>), LUA_ARGS(lc::AbstractDocument * doc))


