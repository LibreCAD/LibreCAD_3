
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "LuaIntf.h"

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
ce = Create(d, "0");
ce:append(l)
ce:execute()
*/

/* Spiral
local r = 10;
local d = 0;
local rx = 100;
local ry = 100;
local p =rx;
local q=ry;

doc=app.currentDocument()
ce = Create(doc, "0");

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
ce:execute()
print "done";
*/

/* Fractal tree
 *

function drawTree( ce, x1,  y1,  angle,  depth)
        if depth == 0 then  return end;

        local x2 = x1 +  (math.cos(math.rad(angle)) * depth * 10.0);
        local y2 = y1 + (math.sin(math.rad(angle)) * depth * 10.0);

       ce:append(Line(Coord(x1, y1), Coord(x2, y2)));
        drawTree(ce, x2, y2, angle - 20, depth - 1);
        drawTree(ce, x2, y2, angle + 20, depth - 1);
end

d=app.currentDocument()
ce = Create(d, "0");
drawTree(ce, 0, 0, -90, 14);
ce:execute()

print "done";


*/


/* Gear
 *
local Gear = {}
Gear.__index = Gear

setmetatable(Gear, {
  __call = function (cls, ...)
    return cls.new(...)
  end,
})

function Gear.new()
  local self = setmetatable({}, Gear)
  return self
end

function Gear:involute_intersect_angle(Rb, R)
    return (math.sqrt(R*R - Rb*Rb) / (Rb)) - (math.acos(Rb / R))
end

function Gear:point_on_circle(radius, angle)
    p = {}
    p.x= radius * math.cos(angle)
    p.y= radius * math.sin(angle)
    return p
end

--ce CreateEntities object
--phi = pressure angle
--PC = Circular Pitch
--teeth = no of teeth
function Gear:calc(ce, N, phi, Pc)
    -- Pitch Circle
    local D = N * Pc / math.pi
    local R = D / 2.0

    -- Diametrial pitch
    local Pd = N / D

    -- Base Circle
    local Db = D * math.cos(phi)
    local Rb = Db / 2.0

    -- Addendum
    local a = 1.0 / Pd

    -- Outside Circle
    local Ro = R + a
    local Do = 2 * Ro

    -- Tooth thickness
    local T = (math.pi * D) / (2 * N)

    -- undercut?
    local U = (2 / (math.sin(phi) * (math.sin(phi))))
    local needs_undercut = N < U
    -- sys.stderr.write("N:%s R:%s Rb:%s\n" % (N,R,Rb))


    -- Clearance
    local c = 0.0
    -- Dedendum
    local b = a + c

    -- Root Circle
    local Rr = R - b
    local Dr = 2 * Rr

    local two_pi = 2 * math.pi
    local half_thick_angle = two_pi / (4 * N)
    local pitch_to_base_angle = self:involute_intersect_angle(Rb, R)
    local pitch_to_outer_angle = self:involute_intersect_angle(Rb, Ro) -- pitch_to_base_angle

    local points ={}
    for x=1,N do
        local c = x * two_pi / N;

        -- angles
        local pitch1 = c - half_thick_angle
        local base1 = pitch1 - pitch_to_base_angle
        local outer1 = pitch1 + pitch_to_outer_angle

        local pitch2 = c + half_thick_angle
        local base2 = pitch2 + pitch_to_base_angle
        local outer2 = pitch2 - pitch_to_outer_angle

        -- points
        local b1 = self:point_on_circle(Rb, base1)
        local p1 = self:point_on_circle(R, pitch1)
        local o1 = self:point_on_circle(Ro, outer1)
        local o2 = self:point_on_circle(Ro, outer2)
        local p2 = self:point_on_circle(R, pitch2)
        local b2 = self:point_on_circle(Rb, base2)

        if Rr >= Rb then
            local pitch_to_root_angle = pitch_to_base_angle - self:involute_intersect_angle(Rb, Rr)
            local root1 = pitch1 - pitch_to_root_angle
            local root2 = pitch2 + pitch_to_root_angle
            local r1 = self:point_on_circle(Rr, root1)
            local r2 = self:point_on_circle(Rr, root2)

            table.insert(points, r1)
            table.insert(points, p1)
            table.insert(points, o1)
            table.insert(points, o2)
            table.insert(points, p2)
            table.insert(points, r2)

        else
            local r1 = self:point_on_circle(Rr, base1)
            local r2 = self:point_on_circle(Rr, base2)
            table.insert(points, r1)
            table.insert(points, b1)
            table.insert(points, p1)
            table.insert(points, o1)
            table.insert(points, o2)
            table.insert(points, p2)
            table.insert(points, b2)
            table.insert(points, r2)
        end
     end
        local first = points[1]
        local fKeep = first
        table.remove(points, 1)
        for k,v in pairs(points) do
            ce:append(Line(Coord(first.x,first.y), Coord(v.x,v.y)))
             first=v
        end
            ce:append(Line(Coord(first.x,first.y), Coord(fKeep.x,fKeep.y)))

end

local gear = Gear()
d=app.currentDocument()
ce = Create(d, "0");
gear:calc(ce, 20,math.rad(10),math.rad(10))
gear:calc(ce, 10,math.rad(10),math.rad(10))
ce:execute()

*/
