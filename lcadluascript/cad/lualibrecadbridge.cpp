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

#include "lua-intf/LuaIntf/LuaIntf.h"
#include "lua-intf/LuaIntf/QtLuaIntf.h"

#include <cad/geometry/geocoordinate.h>
#include <cad/primitive/line.h>
#include <cad/primitive/circle.h>
#include <cad/document/abstractdocument.h>
#include <cad/dochelpers/documentimpl.h>
#include <cad/operations/builder.h>
#include <cad/operations/builderops.h>

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(boost::shared_ptr)
}

using namespace LuaIntf;
using namespace lc;

void lua_openlckernel(lua_State* L) {

    LuaBinding(L)
    .beginClass <operation::DocumentOperation> ("Operation")
    .addFunction("execute", &operation::DocumentOperation::execute)
    .endClass()

    .beginClass <ID> ("ID")
    .addFunction("id", &ID::id)
    .endClass()

    .beginExtendClass <CADEntity, ID> ("CADEntity")
    .endClass()

    .beginClass <geo::Vector> ("Vector")
    .addConstructor(LUA_SP(shared_ptr<const geo::Vector>), LUA_ARGS(const geo::Coordinate & start, const geo::Coordinate & end))
    .endClass()

    .beginClass <AbstractDocument> ("AbstractDocument")
    .endClass()

    .beginClass<geo::Coordinate>("Coord")
    .addConstructor(LUA_ARGS(double x, double y))
    .addFunction("x", &geo::Coordinate::x)
    .addFunction("y", &geo::Coordinate::y)
    .addFunction("z", &geo::Coordinate::z)
    .endClass()

    .beginClass<geo::Coordinate>("Coordinate")
    .addConstructor(LUA_ARGS(double x, double y))
    .addFunction("x", &geo::Coordinate::x)
    .addFunction("y", &geo::Coordinate::y)
    .addFunction("z", &geo::Coordinate::z)
    .endClass()

    .beginExtendClass <Line, CADEntity> ("Line")
    .addConstructor(LUA_SP(shared_ptr< Line>), LUA_ARGS(const geo::Coordinate & start, const geo::Coordinate & end))
    .endClass()

    .beginExtendClass <Circle, CADEntity> ("Circle")
    .addConstructor(LUA_SP(shared_ptr< Circle>), LUA_ARGS(const geo::Coordinate & center, double radius))
    .endClass()

    .beginClass <DocumentImpl> ("DocumentImpl")
    .endClass()

    .beginClass <QString> ("QString")
    .addConstructor(LUA_ARGS(const char*))
    .endClass()

    .beginExtendClass <LuaBuilderProxy, lc::operation::DocumentOperation> ("Builder")
    .addConstructor(LUA_SP(shared_ptr<LuaBuilderProxy>), LUA_ARGS(lc::AbstractDocument * doc))
    .addFunction("append", &LuaBuilderProxy::append)
    .addFunction("move", &LuaBuilderProxy::move)
    .addFunction("copy", &LuaBuilderProxy::copy)
    .addFunction("rotate", &LuaBuilderProxy::rotate)
    .addFunction("push", &LuaBuilderProxy::push)
    .addFunction("loop", &LuaBuilderProxy::repeat)
    .addFunction("begin", &LuaBuilderProxy::begin)
    .endClass()

    .beginClass <lc::operation::Base> ("Base")
    .endClass()
    .beginExtendClass <lc::operation::Move, lc::operation::Base> ("Move")
    .addConstructor(LUA_SP(shared_ptr< lc::operation::Move>), LUA_ARGS(const lc::geo::Coordinate & offset))
    .endClass()
    .beginExtendClass <lc::operation::Begin, lc::operation::Base> ("Begin")
    .addConstructor(LUA_SP(shared_ptr< lc::operation::Begin>), LUA_ARGS())
    .endClass()
    .beginExtendClass <lc::operation::Loop, lc::operation::Base> ("Loop")
    .addConstructor(LUA_SP(shared_ptr< lc::operation::Loop>), LUA_ARGS(const int numTimes))
    .endClass()
    .beginExtendClass <lc::operation::Copy, lc::operation::Base> ("Copy")
    .addConstructor(LUA_SP(shared_ptr< lc::operation::Copy>), LUA_ARGS(const lc::geo::Coordinate & offset))
    .endClass()
    .beginExtendClass <lc::operation::Push, lc::operation::Base> ("Push")
    .addConstructor(LUA_SP(shared_ptr< lc::operation::Push>), LUA_ARGS())
    .endClass();

}

// .addConstructor(LUA_SP(shared_ptr<const CreateEntities>), LUA_ARGS(AbstractDocument * doc))


