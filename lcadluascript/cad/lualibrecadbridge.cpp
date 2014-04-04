extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}
#include "LuaBridge/LuaBridge.h"


#include "lualibrecadbridge.h"


#include <cad/geometry/geocoordinate.h>
#include <cad/primitive/line.h>
#include <cad/document/abstractdocument.h>
#include <cad/dochelpers/documentimpl.h>
#include <cad/operations/createentities.h>





void lua_openlckernel(lua_State* L)
{
    luabridge::getGlobalNamespace (L)
            .beginNamespace ("lc")
            .beginClass <lc::geo::Coordinate> ("Coordinate")
            .addConstructor <void (*) (double x, double y)>()
            .addFunction ("x", &lc::geo::Coordinate::x)
            .addFunction ("y", &lc::geo::Coordinate::y)
            .addFunction ("z", &lc::geo::Coordinate::z)
            .endClass ()
            .endNamespace ();
    luabridge::getGlobalNamespace (L)
            .beginNamespace ("lc")
            .beginClass <lc::geo::Coordinate> ("Coord")
            .addConstructor <void (*) (double x, double y)>()
            .addFunction ("x", &lc::geo::Coordinate::x)
            .addFunction ("y", &lc::geo::Coordinate::y)
            .addFunction ("z", &lc::geo::Coordinate::z)
            .endClass ()
            .endNamespace ();

    luabridge::getGlobalNamespace (L)
            .beginNamespace ("lc")
            .beginClass <lc::DocumentImpl> ("Document")
            .addConstructor <void (*) (void)>()
            .addFunction ("operateOn", &lc::DocumentImpl::operateOn)
            .endClass()
            .endNamespace();

    luabridge::getGlobalNamespace (L)
            .beginNamespace ("lc")
            .beginClass <lc::Line> ("Line")
            .addConstructor <void (*) (const lc::geo::Coordinate start, const lc::geo::Coordinate end)>()
            .endClass()
            .endNamespace();

    luabridge::getGlobalNamespace (L)
            .beginNamespace ("lc")
            .beginClass <lc::CreateEntities> ("CreateEntities")
            .addConstructor <void (*) (lc::AbstractDocument* document, const QString& layerName)>()
            .addFunction ("append", &lc::CreateEntities::append)
            .endClass()
            .endNamespace();
}
