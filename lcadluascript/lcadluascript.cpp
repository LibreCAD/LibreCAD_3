
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/RefCountedObject.h"
#include "LuaBridge/RefCountedPtr.h"
#include "lcadluascript.h"
#include "cad/lualibrecadbridge.h"

#include <cad/dochelpers/documentimpl.h>

// https://github.com/pisto/spaghettimod/commits/master/include/
// https://bitbucket.org/alexames/luawrapperexample/overview
// https://github.com/jeremyong/Selene
// https://github.com/vinniefalco/LuaBridge -> fixes https://github.com/pisto/spaghettimod/commits/master/include/
// http://www.rasterbar.com/products/luabind.html

LCadLuaScript::LCadLuaScript(lc::AbstractDocument* document)
{
    _document = document;
}

QString *gOut;
lc::AbstractDocument* luaDoc;


static int l_my_print(lua_State* L) {
    int nargs = lua_gettop(L);
    for (int i=1; i <= nargs; ++i) {
        gOut->append(lua_tostring(L, i));
        gOut->append("\n");
    }
    return 0;
}

static const struct luaL_Reg printlib [] = {
    {"print", l_my_print},
    {NULL, NULL}
};

RefCountedPtr<lc::AbstractDocument> getDocument()
{
    return RefCountedPtr<lc::AbstractDocument>(luaDoc);
}


QString LCadLuaScript::run(const QString &script) {

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // add lua cad entities
    lua_openlckernel(L);

    luabridge::getGlobalNamespace(L)
      .addFunction("Document",&getDocument);

    // push the document to lua


    // Other lua stuff
    lua_getglobal(L, "_G");
    luaL_setfuncs(L, printlib, 0);
    lua_pop(L, 1);

    // Some globals we have to figure out to make sure it works with multiple threads
    QString out; gOut = &out;
    luaDoc = _document;

    // luaL_dofile(L, "/opt/librecad-test.lua");
    int s = luaL_dostring(L, script.toLocal8Bit().data());
    if (s!=0) {
        out.append(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    lua_close(L);
    return out;
}

/*
 * c = lc.Coordinate(12.12,23);
l = lc.Line(lc.Coord(12.12,23), lc.Coord(12.12,23));
doc= lc.Document();
if not doc then
print "empty";
end
print (c:x());
print "foo";
*/
