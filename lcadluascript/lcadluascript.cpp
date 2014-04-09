
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "lua-intf/LuaIntf.h"

#include "lcadluascript.h"
#include "cad/lualibrecadbridge.h"

#include <cad/dochelpers/documentimpl.h>

using namespace LuaIntf;

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
lc::DocumentImpl* luaDoc;


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

static lc::AbstractDocument * lua_getDocument() {
    return luaDoc;
}

QString LCadLuaScript::run(const QString &script) {

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // add lua cad entities
    lua_openlckernel(L);

    LuaBinding(L).beginModule("app")
            .addFunction("currentDocument", &lua_getDocument);

    // Other lua stuff
    lua_getglobal(L, "_G");
    luaL_setfuncs(L, printlib, 0);
    lua_pop(L, 1);

    // Some globals we have to figure out to make sure it works with multiple threads
    QString out; gOut = &out;
    luaDoc = static_pointer_cast<lc::DocumentImpl>(_document);

    // luaL_dofile(L, "/opt/librecad-test.lua");
    int s = luaL_dostring(L, script.toLocal8Bit().data());
    if (s!=0) {
        out.append(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    lua_close(L);
    return out;
}

/* Line
 l=Line(Coord(0,0), Coord(10,100));
d=app.currentDocument()
ce = CreateEntities(d, "0");
ce:append(l)
d:operateOn(ce)
*/

/* Spiral
local r = 10;
local d = 0;
local rx = 100;
local ry = 100;
local p =rx;
local q=ry;

doc=app.currentDocument()
ce = CreateEntities(doc, "0");

while (d< 8*math.pi) do
    local x=rx+(math.sin(d)*d)*r;
    local y=ry+(math.sin(d+(math.pi/2))*(d+(math.pi/2)) * r);
    if (d>0) then
        ce:append(Line(Coord(x,y), Coord(p,q)));
    end
    p=x;
    q=y;
    d=d + 0.01;
end
doc:operateOn(ce)
print "done";
*/

/* Fractal tree
 *
local numLines = 0;

function drawTree( ce, x1,  y1,  angle,  depth)
        if depth == 0 then  return end;

        local x2 = x1 +  (math.cos(math.rad(angle)) * depth * 10.0);
        local y2 = y1 + (math.sin(math.rad(angle)) * depth * 10.0);

       ce:append(Line(Coord(x1, y1), Coord(x2, y2)));
numLines=numLines+1;
        drawTree(ce, x2, y2, angle - 20, depth - 1);
        drawTree(ce, x2, y2, angle + 20, depth - 1);
end

d=app.currentDocument()
ce = CreateEntities(d, "0");
drawTree(ce, 400, 500, -90, 12);
d:operateOn(ce)

print ("Lines " .. numLines);

*/
