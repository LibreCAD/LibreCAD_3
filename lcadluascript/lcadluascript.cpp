extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include <iostream>
#include "LuaIntf.h"

#include "lcadluascript.h"
#include "cad/lualibrecadbridge.h"
#include "cad/timer.h"
#include "cad/base/metainfo.h"

#include <type_traits>

#include <cad/dochelpers/documentimpl.h>

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
}

using namespace LuaIntf;

// https://github.com/pisto/spaghettimod/commits/master/include/
// https://bitbucket.org/alexames/luawrapperexample/overview
// https://github.com/jeremyong/Selene
// https://github.com/vinniefalco/LuaBridge -> fixes https://github.com/pisto/spaghettimod/commits/master/include/
// http://www.rasterbar.com/products/luabind.html

LCadLuaScript::LCadLuaScript(std::shared_ptr<lc::Document> document) : _document(document), _usePrintLib(true) {
}

LCadLuaScript::LCadLuaScript(std::shared_ptr<lc::Document> document, bool usePrintLib) : _document(document), _usePrintLib(usePrintLib) {
}

std::string* gOut;
std::shared_ptr<lc::Document> luaDoc;

static int l_my_print(lua_State* L) {
    int nargs = lua_gettop(L);

    for (int i = 1; i <= nargs; ++i) {
        const char * str = lua_tostring(L, i);
        if (str!=nullptr) {
            gOut->append(str);
        } else {
            gOut->append("lc: lua_tostring was null");
        }
        gOut->append("\n");
    }

    return 0;
}

static const struct luaL_Reg printlib[] = {
    {"print", l_my_print},
    {nullptr, nullptr}
};

static std::shared_ptr<lc::Document> lua_getDocument() {
    return luaDoc;
}

static lc::Layer_SPtr lua_layer(const char* layer) {
    // Cast until the lua bridge understands Layer_CSPtr as a return value
    return std::const_pointer_cast<lc::Layer>(luaDoc->layerByName(layer));

}

/**********************************************************************************************/
/* Functions below are helper function's because luaintf (at least I couldn't get it working **/
/* didn't understand the default values on constructors, even when using the _opt<> macro's  **/
/* All helper's ending with 1 have the same constructor except metaInfo added                **/
/**********************************************************************************************/
static lc::entity::Line_SPtr lua_line1(const lc::geo::Coordinate& start, const lc::geo::Coordinate& end, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::Line>(start, end, layer, metaInfo);
}
static lc::entity::Circle_SPtr lua_circle1(const lc::geo::Coordinate& center, double radius, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::Circle>(center, radius, layer, metaInfo);
}
static lc::entity::Arc_SPtr lua_arc1(const lc::geo::Coordinate& center, double radius, double startAngle, double endAngle, bool CCW, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::Arc>(center, radius, startAngle, endAngle, CCW, layer, metaInfo);
}
static lc::entity::Point_SPtr lua_point(const lc::geo::Coordinate& center, const lc::Layer_CSPtr layer) {
    return std::make_shared<lc::entity::Point>(center, layer, nullptr);
}
static lc::entity::Point_SPtr lua_point1(const lc::geo::Coordinate& center, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::Point>(center, layer, metaInfo);
}

/***
*    |~\. _ _ |~) _  _|. _ |
*    |_/|| | ||~\(_|(_||(_||
*
*/
static lc::entity::DimRadial_SPtr lua_dimRadial(lc::geo::Coordinate const& definitionPoint, lc::geo::Coordinate const& middleOfText, int const attachmentPoint, double angle, double const lineSpacingFactor,
                                        const int lineSpacingStyle, std::string const& explicitValue,
                                        lc::geo::Coordinate const& definitionPoint2, const double leader, const lc::Layer_CSPtr layer) {
    return std::make_shared<lc::entity::DimRadial>(definitionPoint, middleOfText, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), angle, lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle), explicitValue,  definitionPoint2, leader, layer);
}
static lc::entity::DimRadial_SPtr lua_dimRadial1(lc::geo::Coordinate const& definitionPoint, lc::geo::Coordinate const& middleOfText, const int attachmentPoint, double angle, double const lineSpacingFactor,
                                         lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                                         lc::geo::Coordinate const& definitionPoint2, const double leader, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::DimRadial>(definitionPoint, middleOfText, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), angle, lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle), explicitValue,  definitionPoint2, leader, layer, metaInfo);
}
static lc::entity::DimRadial_SPtr lua_dimRadial2(lc::geo::Coordinate const& definitionPoint, const int attachmentPoint, double const lineSpacingFactor,
                                         lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                                         lc::geo::Coordinate const& definitionPoint2, const double leader, const lc::Layer_CSPtr layer) {
    return std::make_shared<lc::entity::DimRadial>(definitionPoint, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle), explicitValue,  definitionPoint2, leader, layer, nullptr);
}
static lc::entity::DimRadial_SPtr lua_dimRadial3(lc::geo::Coordinate const& definitionPoint, const int attachmentPoint, double const lineSpacingFactor,
                                         lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                                         lc::geo::Coordinate const& definitionPoint2, const double leader, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::DimRadial>(definitionPoint, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle), explicitValue,  definitionPoint2, leader, layer, metaInfo);
}


/***
*    |~\. _ _ |~\. _  _ _  _ _|_ _. _
*    |_/|| | ||_/|(_|| | |(/_ | | |(_
*
*/
static lc::entity::DimDiametric_SPtr lua_dimDiametric(lc::geo::Coordinate const& definitionPoint, lc::geo::Coordinate const& middleOfText, int const attachmentPoint, double angle, double const lineSpacingFactor,
                                              const int lineSpacingStyle, std::string const& explicitValue,
                                              lc::geo::Coordinate const& definitionPoint2, const double leader, const lc::Layer_CSPtr layer) {
    return std::make_shared<lc::entity::DimDiametric>(definitionPoint, middleOfText, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), angle, lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle), explicitValue,  definitionPoint2, leader, layer);
}
static lc::entity::DimDiametric_SPtr lua_dimDiametric1(lc::geo::Coordinate const& definitionPoint, lc::geo::Coordinate const& middleOfText, const int attachmentPoint, double angle, double const lineSpacingFactor,
                                               lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                                               lc::geo::Coordinate const& definitionPoint2, const double leader, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::DimDiametric>(definitionPoint, middleOfText, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), angle, lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle), explicitValue,  definitionPoint2, leader, layer, metaInfo);
}
static lc::entity::DimDiametric_SPtr lua_dimDiametric2(lc::geo::Coordinate const& definitionPoint, const int attachmentPoint, double const lineSpacingFactor,
                                               lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                                               lc::geo::Coordinate const& definitionPoint2, const double leader, const lc::Layer_CSPtr layer) {
    return std::make_shared<lc::entity::DimDiametric>(definitionPoint, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle), explicitValue,  definitionPoint2, leader, layer, nullptr);
}
static lc::entity::DimDiametric_SPtr lua_dimDiametric3(lc::geo::Coordinate const& definitionPoint, const int attachmentPoint, double const lineSpacingFactor,
                                               lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                                               lc::geo::Coordinate const& definitionPoint2, const double leader, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::DimDiametric>(definitionPoint, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle), explicitValue,  definitionPoint2, leader, layer, metaInfo);
}


/***
*    |~\. _ _ | . _  _  _  _
*    |_/|| | ||_|| |(/_(_||
*
*/
static lc::entity::DimLinear_SPtr lua_dimLinear(lc::geo::Coordinate const& definitionPoint, lc::geo::Coordinate const& middleOfText, int const attachmentPoint, double textAngle, double const lineSpacingFactor,
                                        const int lineSpacingStyle, std::string const& explicitValue,
                                        lc::geo::Coordinate const& definitionPoint2, lc::geo::Coordinate const& definitionPoint3, const double angle, const double oblique, const lc::Layer_CSPtr layer) {
    return std::make_shared<lc::entity::DimLinear>(definitionPoint, middleOfText, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), textAngle, lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle), explicitValue, definitionPoint2, definitionPoint3, angle, oblique, layer, nullptr);
}
static lc::entity::DimLinear_SPtr lua_dimLinear1(lc::geo::Coordinate const& definitionPoint, lc::geo::Coordinate const& middleOfText, const int attachmentPoint, double textAngle, double const lineSpacingFactor,
                                         lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                                         lc::geo::Coordinate const& definitionPoint2, lc::geo::Coordinate const& definitionPoint3, const double angle, const double oblique, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::DimLinear>(definitionPoint, middleOfText, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), textAngle, lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle), explicitValue, definitionPoint2, definitionPoint3, angle, oblique, layer, metaInfo);
}
static lc::entity::DimLinear_SPtr lua_DimLinear_dimAuto(lc::geo::Coordinate const& p1, lc::geo::Coordinate const& p2, double const textOffset, std::string const& explicitValue, const lc::Layer_CSPtr layer) {
    return lc::entity::DimLinear::dimAuto(p1, p2, textOffset, explicitValue, layer, nullptr);
}
static lc::entity::DimLinear_SPtr lua_DimLinear_dimAuto1(lc::geo::Coordinate const& p1, lc::geo::Coordinate const& p2, double const textOffset, std::string const& explicitValue, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return lc::entity::DimLinear::dimAuto(p1, p2, textOffset, explicitValue, layer, metaInfo);
}

/***
*    |~\. _ _  /\ |. _  _  _  _|
*    |_/|| | |/~~\||(_|| |(/_(_|
*                    _|
*/
static lc::entity::DimAligned_SPtr lua_dimAligned(lc::geo::Coordinate const& definitionPoint, lc::geo::Coordinate const& middleOfText, lc::TextConst::AttachmentPoint const& attachmentPoint, double textAngle, double const lineSpacingFactor,
                                          lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                                          lc::geo::Coordinate const& definitionPoint2,  lc::geo::Coordinate const& definitionPoint3, const lc::Layer_CSPtr layer) {
    return std::make_shared<lc::entity::DimAligned>(definitionPoint, middleOfText, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), textAngle, lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle),
                                            explicitValue, definitionPoint2, definitionPoint3, layer, nullptr);
}
static lc::entity::DimAligned_SPtr lua_dimAligned1(lc::geo::Coordinate const& definitionPoint, lc::geo::Coordinate const& middleOfText, lc::TextConst::AttachmentPoint const& attachmentPoint, double textAngle, double const lineSpacingFactor,
                                           lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                                           lc::geo::Coordinate const& definitionPoint2,  lc::geo::Coordinate const& definitionPoint3, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::DimAligned>(definitionPoint, middleOfText, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), textAngle, lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle),
                                            explicitValue, definitionPoint2, definitionPoint3, layer, metaInfo);
}

static lc::entity::DimAligned_SPtr lua_DimAligned_dimAuto(lc::geo::Coordinate const& p1, lc::geo::Coordinate const& p2, double const textOffset, std::string const& explicitValue, const lc::Layer_CSPtr layer) {
    return lc::entity::DimAligned::dimAuto(p1, p2, textOffset, explicitValue, layer, nullptr);
}
static lc::entity::DimAligned_SPtr lua_DimAligned_dimAuto1(lc::geo::Coordinate const& p1, lc::geo::Coordinate const& p2, double const textOffset, std::string const& explicitValue, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return lc::entity::DimAligned::dimAuto(p1, p2, textOffset, explicitValue, layer, metaInfo);
}
/***
*    |~\. _ _  /\  _  _    | _  _
*    |_/|| | |/~~\| |(_||_||(_||
*                     _|
*/
static lc::entity::DimAngular_SPtr lua_dimAngular(lc::geo::Coordinate const& definitionPoint, lc::geo::Coordinate const& middleOfText, lc::TextConst::AttachmentPoint const& attachmentPoint, double textAngle, double const lineSpacingFactor,
                                          lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                                          lc::geo::Coordinate const& defPoint11, lc::geo::Coordinate const& defPoint12, lc::geo::Coordinate const& defPoint21, lc::geo::Coordinate const& defPoint22, const lc::Layer_CSPtr layer) {
    return std::make_shared<lc::entity::DimAngular>(definitionPoint, middleOfText, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), textAngle, lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle),
                                            explicitValue, defPoint11, defPoint12, defPoint21, defPoint22, layer, nullptr);
}
static lc::entity::DimAngular_SPtr lua_dimAngular1(lc::geo::Coordinate const& definitionPoint, lc::geo::Coordinate const& middleOfText, lc::TextConst::AttachmentPoint const& attachmentPoint, double textAngle, double const lineSpacingFactor,
        lc::TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
        lc::geo::Coordinate const& defPoint11, lc::geo::Coordinate const& defPoint12, lc::geo::Coordinate const& defPoint21, lc::geo::Coordinate const& defPoint22, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::DimAngular>(definitionPoint, middleOfText, static_cast<lc::TextConst::AttachmentPoint>(attachmentPoint), textAngle, lineSpacingFactor, static_cast<lc::TextConst::LineSpacingStyle>(lineSpacingStyle),
            explicitValue, defPoint11, defPoint12, defPoint21, defPoint22, layer, metaInfo);
}
/***
*    (~ _ |. _  _
*    _)|_)||| |(/_
*      |
*/
static lc::entity::Spline_SPtr lua_spline(LuaRef luaControlPoints,
        LuaRef luaKnotPoints,
        LuaRef luaFitPoints,
        int degree, bool closed, double fitTolerance,
        double stanx, double stany, double stanz,
        double etanx, double etany, double etanz,
        double nx, double ny, double nz,
        const lc::Layer_CSPtr layer) {
    std::vector<lc::geo::Coordinate> controlPoints = Lua::getList<std::vector<lc::geo::Coordinate>>(luaControlPoints);
    std::vector<double> knotPoints = Lua::getList<std::vector<double>>(luaKnotPoints);
    std::vector<lc::geo::Coordinate> fitPoints = Lua::getList<std::vector<lc::geo::Coordinate>>(luaFitPoints);
    return std::make_shared<lc::entity::Spline>(controlPoints, knotPoints, fitPoints, degree, closed, fitTolerance, stanx, stany, stanz, etanx, etany, etanz, nx, ny, nz, layer, nullptr);
}

static lc::entity::Spline_SPtr lua_spline1(LuaRef luaControlPoints,
        LuaRef luaKnotPoints,
        LuaRef luaFitPoints,
        int degree, bool closed, double fitTolerance,
        double stanx, double stany, double stanz,
        double etanx, double etany, double etanz,
        double nx, double ny, double nz,
        const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    std::vector<lc::geo::Coordinate> controlPoints = Lua::getList<std::vector<lc::geo::Coordinate>>(luaControlPoints);
    std::vector<double> knotPoints = Lua::getList<std::vector<double>>(luaKnotPoints);
    std::vector<lc::geo::Coordinate> fitPoints = Lua::getList<std::vector<lc::geo::Coordinate>>(luaFitPoints);
    return std::make_shared<lc::entity::Spline>(controlPoints, knotPoints, fitPoints, degree, closed, fitTolerance, stanx, stany, stanz, etanx, etany, etanz, nx, ny, nz, layer, metaInfo);
}

/***
 *    |\    /|~) _ |  |. _  _
 *    |_\/\/ |~ (_)|\/||| |(/_
 *                  /
 */
static lc::entity::LWPolyline_SPtr lua_lwpolyline(LuaRef lrefvertex, double width, double elevation, double thickness, bool closed, const lc::geo::Coordinate& extrucsion_direction, const lc::Layer_CSPtr layer) {
    std::vector<lc::entity::LWVertex2D> vertex = Lua::getList<std::vector<lc::entity::LWVertex2D>>(lrefvertex);

    return std::make_shared<lc::entity::LWPolyline>(vertex, width, elevation, thickness, closed, extrucsion_direction, layer);
}

static lc::entity::LWPolyline_SPtr lua_lwpolyline1(LuaRef lrefvertex, double width, double elevation, double thickness, bool closed, const lc::geo::Coordinate& extrucsion_direction, const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    std::vector<lc::entity::LWVertex2D> vertex = Lua::getList<std::vector<lc::entity::LWVertex2D>>(lrefvertex);

    return std::make_shared<lc::entity::LWPolyline>(vertex, width, elevation, thickness, closed, extrucsion_direction, layer, metaInfo);
}


/***
*    ~|~ _  _|_
*     | (/_><|
*
*/static lc::entity::Text_SPtr lua_text(const lc::geo::Coordinate& insertion_point,
                                const std::string text_value,
                                const double height,
                                const double angle,
                                const std::string style,
                                const int textgeneration,
                                const int halign,
                                const int valign,
                                const lc::Layer_CSPtr layer) {
    return std::make_shared<lc::entity::Text>(insertion_point, text_value, height, angle, style, static_cast<lc::TextConst::DrawingDirection>(textgeneration), static_cast<lc::TextConst::HAlign>(halign), static_cast<lc::TextConst::VAlign>(valign), layer);
}
static lc::entity::Text_SPtr lua_text1(const lc::geo::Coordinate& insertion_point,
                               const std::string text_value,
                               const double height,
                               const double angle,
                               const std::string style,
                               const int textgeneration,
                               const int halign,
                               const int valign,
                               const lc::Layer_CSPtr layer,
                               const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::Text>(insertion_point, text_value, height, angle, style, static_cast<lc::TextConst::DrawingDirection>(textgeneration), static_cast<lc::TextConst::HAlign>(halign), static_cast<lc::TextConst::VAlign>(valign), layer, metaInfo);
}


/*
static lc::entity::DimLinear_SPtr lua_DimLinear1(const lc::entity::Dimension& dimension,
        const lc::geo::Coordinate& extension_point1,
        const lc::geo::Coordinate& extension_point2,
        const double oblique,
        const lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo) {
    return std::make_shared<lc::entity::DimLinear>(extension_point1, extension_point2, oblique, layer, metaInfo);
}*/




std::string LCadLuaScript::run(const std::string& script) {

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // add lua cad entities
    lua_openlckernel(L);

    LuaBinding(L)
    .addFunction("Line1", &lua_line1)
    .addFunction("Circle1", &lua_circle1)
    .addFunction("Arc1", &lua_arc1)
    .addFunction("Point", &lua_point)
    .addFunction("Point1", &lua_point1)
    .addFunction("Text", &lua_text)
    .addFunction("Text1", &lua_text1)
    .addFunction("Spline", &lua_spline)
    .addFunction("Spline1", &lua_spline1)
    .addFunction("DimRadial", &lua_dimRadial)
    .addFunction("DimRadial1", &lua_dimRadial1)
    .addFunction("DimRadial2", &lua_dimRadial2)
    .addFunction("DimRadial3", &lua_dimRadial3)
    .addFunction("DimDiametric", &lua_dimDiametric)
    .addFunction("DimDiametric1", &lua_dimDiametric1)
    .addFunction("DimDiametric2", &lua_dimDiametric2)
    .addFunction("DimDiametric3", &lua_dimDiametric3)
    .addFunction("DimLinear", &lua_dimLinear)
    .addFunction("DimLinear1", &lua_dimLinear1)
    .addFunction("DimLinear_DimAuto", &lua_DimLinear_dimAuto)
    .addFunction("DimLinear_DimAuto1", &lua_DimLinear_dimAuto1)
    .addFunction("DimAligned", &lua_dimAligned)
    .addFunction("DimAligned1", &lua_dimAligned1)
    .addFunction("DimAligned_DimAuto", &lua_DimAligned_dimAuto)
    .addFunction("DimAligned_DimAuto1", &lua_DimAligned_dimAuto1)
    .addFunction("DimAngular", &lua_dimAngular)
    .addFunction("DimAngular1", &lua_dimAngular1)
    .addFunction("LWPolyline", &lua_lwpolyline)
    .addFunction("LWPolyline1", &lua_lwpolyline1)
    .beginModule("active")
    .addFunction("document", &lua_getDocument)
    .beginModule("proxy")
    .addFunction("layerByName", &lua_layer)
    .endModule()
    .endModule();

    LuaBinding(L)
    .addFunction("microtime", &lua_microtime);

    // Other lua stuff
    lua_getglobal(L, "_G");

    if (_usePrintLib) {
        luaL_setfuncs(L, printlib, 0);
    }

    lua_pop(L, 1);

    // Some globals we have to figure out to make sure it works with multiple threads
    std::string out;
    gOut = &out;
    luaDoc = _document;

    int s = luaL_dostring(L, script.c_str());

    if (s != 0) {
        out.append(lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    lua_close(L);
    return out;
}


/*Point
layer = active.proxy.layerByName("0")
p=Point(Coord(50,50), layer);
d=active.document()
Builder(d):append(p):execute()
*/

/* Dim Angular
layer = active.proxy.layerByName("0")
dr1=DimAngular(Coord(50,50), Coord(100,100), 2, 0, 1., 1, "<>",
Coord(30,100), Coord(0,10),
Coord(10,0), Coord(100,30), layer);
d=active.document()
Builder(d):append(dr1):execute()
*/

/* DimRadial
layer = active.proxy.layerByName("0")
dr1=DimRadial(Coord(50,50), Coord(120,120), 2, 0, 1., 1, "<>", Coord(100,100), 0., layer);
dr2=DimRadial(Coord(50,50), Coord(25,50), 2, 0, 1., 1, "Explicit", Coord(-70.71+50,50), 0., layer);
dr3=DimRadial2(Coord(50,50), 2, 1., 1, "<>", Coord(0,0), 0., layer);
dr4=DimRadial2(Coord(50,50), 2, 1., 1, "<>", Coord(50,0), 0., layer);
dr5=DimRadial2(Coord(50,50), 2, 1., 1, "<>", Coord(100,0), 0., layer);
dr6=DimRadial2(Coord(50,50), 2, 1., 10, "<>", Coord(0,100), 25., layer);
c=Circle(Coord(50, 50), 70.7106781, layer);
d=active.document()
Builder(d):append(dr1):append(dr2):append(dr3):append(dr4):append(dr5):append(dr6):append(c):execute()
 */

/* DimDiametric
layer = active.proxy.layerByName("0")
dr1=DimDiametric(Coord(0,100), Coord(130,-30), 2, 0, 1., 1, "<>", Coord(100,0), 0., layer);
dr2=DimDiametric2(Coord(0,0), 2, 1., 1, "<>", Coord(100,100), 0., layer);
dr3=DimDiametric2(Coord(0,0), 2, 1., 1, "<>", Coord(5,5), 0., layer);
c3=Circle(Coord(2.5,2.5), 1.8*2, layer);
c=Circle(Coord(50, 50), 70.7106781, layer);
d=active.document()
Builder(d):append(dr1):append(dr2):append(dr3):append(c):append(c3):execute()
*/


/* DimLinear
layer = active.proxy.layerByName("0")
dr1=DimLinear_DimAuto(Coord(0,50), Coord(100,30), 0, "<>", layer);
dr2=DimLinear_DimAuto(Coord(-50,50), Coord(-75,-75), 0, "<>", layer);
dr3=DimLinear_DimAuto(Coord(25,-50), Coord(125,-50), -25, "<>", layer);
dr4=DimLinear_DimAuto(Coord(15,-50), Coord(15,-150), -25, "<>", layer);
d=active.document()
Builder(d):append(dr1):append(dr2):append(dr3):append(dr4):execute()
*/

/* Dim Aligned
layer = active.proxy.layerByName("0")
dr1=DimAligned_DimAuto(Coord(100,100), Coord(0,0), 10, "<>", layer);
dr2=DimAligned_DimAuto(Coord(0,0), Coord(100,0), 10, "<>", layer);
dr3=DimAligned_DimAuto(Coord(-25,-25), Coord(-130,0), 15, "<>", layer);
d=active.document()
Builder(d):append(dr1):append(dr2):append(dr3):execute()
*/

/* Line
layer = active.proxy.layerByName("0")
l=Line(Coord(0,0), Coord(10,100), layer);
d=active.document()
Builder(d):append(l):execute()
*/


/* Arc
layer = active.proxy.layerByName("0")
a=Arc(Coord(0,0), 10, 0.25*math.pi,1.5*math.pi, layer);
d=active.document()
Builder(d):append(a):execute()
*/

/*
layer=Layer("10", Color(1,0,0,0));
al = AddLayer(active.document(), layer);
al:execute();
l=Line(Coord(0,0), Coord(10,100), layer);
d=active.document()
Builder(d):append(l):execute()
 */

/* Circle
layer = active.proxy.layerByName("0")
c=Circle(Coord(50, 50), 70.7106781, layer);
d=active.document()
Builder(d):append(c):execute()
*/


/*
layer = active.proxy.layerByName("0");
metaInfo1= MetaInfo():add(MetaLineWidth(1));
metaInfo2 = MetaInfo():add(MetaLineWidth(10));
metaInfo2:add(MetaColor(Color(0,1,0,1)));
l1=Line1(Coord(-100,-100), Coord(100,100), layer, metaInfo1);
l2=Line1(Coord(-100,100), Coord(100,-100), layer, metaInfo2);
d=active.document();
Builder(d):append(l1):append(l2):execute();

 */


/*
layer = active.proxy.layerByName("0");
metaInfo1= MetaInfo():add(MetaLineWidth(1));
metaInfo2 = MetaInfo():add(MetaLineWidth(2));
l1=Line1(Coord(-100,-100), Coord(100,100), layer, metaInfo1);
l2=Line1(Coord(-100,100), Coord(100,-100), layer, metaInfo2);
d=active.document();
Builder(d):append(l1):append(l2):execute();

 */

/* Spiral
start = microtime()
local r = 10;
local d = 0;
local rx = 100;
local ry = 100;
local p =rx;
local q=ry;

doc=active.document()
ce=Builder(doc,em)
layer = active.proxy.layerByName("0")
local count=0
while (d< 8*math.pi) do
    local x=rx+(math.sin(d)*d)*r;
    local y=ry+(math.sin(d+(math.pi/2))*(d+(math.pi/2)) * r);
    if (d>0) then
        ce:append(Line(Coord(x,y), Coord(p,q),layer));
    end
    p=x;
    q=y;
    d=d + 0.005;
   count=count + 1
end
ce:execute()
print "done";
print (count)
print (microtime()-start);

*/

/* Fractal tree
 *
start = microtime()
layer = active.proxy.layerByName("0")
function drawTree( ce, x1,  y1,  angle,  depth)
        if depth == 0 then  return end;

        local x2 = x1 +  (math.cos(math.rad(angle)) * depth * 10.0);
        local y2 = y1 + (math.sin(math.rad(angle)) * depth * 10.0);


       ce:append(Line(Coord(x1, y1), Coord(x2, y2),layer));
        drawTree(ce, x2, y2, angle - 20, depth - 1);
        drawTree(ce, x2, y2, angle + 20, depth - 1);
end

doc=active.document()
ce=Builder(doc,em)
drawTree(ce, 0, 0, 90, 14);
ce:execute()

print (microtime()-start);


*/


/* Gear
 *
layer = active.proxy.layerByName("0")

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
function Gear:calc(builder, N, phi, Pc)
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
            builder:append(Line(Coord(first.x,first.y), Coord(v.x,v.y),layer))
             first=v
        end
            builder:append(Line(Coord(first.x,first.y), Coord(fKeep.x,fKeep.y),layer))

end

local gear = Gear()

b=Builder(active.document())
layer = active.proxy.layerByName("0")

gear:calc(b, 20,math.rad(10),math.rad(10))
gear:calc(b, 10,math.rad(10),math.rad(10))
b:push()
b:scale(Coord(0,0),Coord(10,10))
b:execute()


*/



/*
 *
layer = active.proxy.layerByName("0")
l=Line(Coord(0,0), Coord(10,100), layer);
d=active.document()
b=Builder(d)
b:append(l)
b:copy(Coord(0,0))
b:rotate(Coord(0,0), math.rad(45))
b:loop(7)
b:execute()
*/


/*
 *
layer = active.proxy.layerByName("0")
l=Line(Coord(0,0), Coord(00,100), layer);
d=active.document()
b=Builder(d)
b:push()
b:append(l)
b:copy(Coord(0,0))
b:rotate(Coord(0,0), math.rad(45))
b:loop(7)
b:push()
b:rotate(Coord(0,0),math.rad(22.5))
b:copy(Coord(150,00))
b:execute()
*/

/*


numEntities=0
function drawTree( ce, x1,  y1,  angle,  depth)
        if depth == 0 then  return end;

        local x2 = x1 +  (math.cos(math.rad(angle)) * depth * 10.0);
        local y2 = y1 + (math.sin(math.rad(angle)) * depth * 10.0);


       ce:append(Line(Coord(x1, y1), Coord(x2, y2),layer));
       numEntities=numEntities+1
        drawTree(ce, x2, y2, angle - 20, depth - 1);
        drawTree(ce, x2, y2, angle + 20, depth - 1);
end

-- Do Create
start = microtime()
treeDepth = 14
doc=active.document()
ce=Builder(doc)
layer = active.proxy.layerByName("0")
drawTree(ce, 0, 0, -0, treeDepth);
layer = active.proxy.layerByName("1")
drawTree(ce, 0, 0, -72, treeDepth);
layer = active.proxy.layerByName("2")
drawTree(ce, 0, 0, -144, treeDepth);
layer = active.proxy.layerByName("3")
drawTree(ce, 0, 0, -216, treeDepth);
layer = active.proxy.layerByName("4")
drawTree(ce, 0, 0, -288, treeDepth);
ce:execute()

print "Creation time"
print (microtime()-start);
print "Number of entities"
print (numEntities)

-- Do a move
start = microtime()
layer = active.proxy.layerByName("0")
d=active.document()
Builder(d):selectByLayer(layer):move(Coord(100,0)):execute()
print "Move time"
print (microtime()-start);

*/

/* Remove
-- Do a remove
start = microtime()
layer = active.proxy.layerByName("1")
d=active.document()
Builder(d):selectByLayer(layer):remove():execute()
print "Remove time"
print (microtime()-start);

*/

/* Scaling
start = microtime()
layer = active.proxy.layerByName("0")
d=active.document()
Builder(d):selectByLayer(layer):scale(Coord(100,0),Coord(2,2)):execute()
print "Move time"
print (microtime()-start);
*/

/*
 *
-- Demo to drawing Text and roations
layer=Layer("10", Color(1,0,0,0));
al = AddLayer(active.document(), layer);
al:execute();

d=active.document()
b = Builder(d)
function render (b_, layer_, text, x, y, a, b)
    local c = 0.0;
    for i=0,360,45 do
    c = math.pi/180 * i;
    b_:append(Text(Coord(x, y), text, 20, c, "standard",0,a,b, layer_));
    end
    b_:append(Circle(Coord(x, y), 5, layer_));
end

render(b, layer, "Top Left", -150, 150, 0, 3);
render(b, layer, "Top Center", 0, 150, 1, 3);
render(b, layer, "Top Right", 150, 150, 2, 3);
render(b, layer, "Middle Left", -150, 0, 0, 2);
render(b, layer, "Middle Center", 0, 0, 1, 2);
render(b, layer, "Middle Right", 150, 0, 2, 2);
render(b, layer, "Bottom Left", -150, -150, 0, 1);
render(b, layer, "Bottom Center", 0, -150, 1, 1);
render(b, layer, "Bottom Right", 150, -150, 2, 1);

-- DRaw suqare (for autoscale)
b:append(Line(Coord(-400,400), Coord(400,400), layer));
b:append(Line(Coord(400,400), Coord(400,-400), layer));
b:append(Line(Coord(400,-400), Coord(-400,-400), layer));
b:append(Line(Coord(-400,-400), Coord(-400,400), layer));

-- Draw center
metaInfo1= MetaInfo():add(MetaColor(Color(1,0,0,1)));
metaInfo2 = MetaInfo():add(MetaColor(Color(0,1,0,1)));
b:append(Line1(Coord(0,0), Coord(0,180), layer, metaInfo1));
b:append(Line1(Coord(0,0), Coord(180,00), layer, metaInfo2));

-- DRaw the thing
b:execute();

*/

/*
layer = active.proxy.layerByName("0")
--Stick Figure
c=Circle(Coord(-50,0), 50, layer); --face
l1=Line(Coord(0,0), Coord(10,100), layer); --torso
l2=Line(Coord(10,100), Coord(-20,150), layer); --right thigh
l3=Line(Coord(10,100), Coord(0,150), layer); --left thigh
l4=Line(Coord(-20,150), Coord(0,210), layer); --right lowerleg
l5=Line(Coord(0,150), Coord(25,210), layer); --left lowerleg
l6=Line(Coord(0,210), Coord(-15,215), layer); --right foot
l7=Line(Coord(25,210), Coord(10,218), layer); --left foot
l8=Line(Coord(2.5,25), Coord(25,60), layer); --left upperarm
l9=Line(Coord(25,60), Coord(10,100), layer); --left lowerarm
l10=Line(Coord(2.5,25), Coord(-10,60), layer); --right upperarm
l11=Line(Coord(-10,60), Coord(10,100), layer); --right lowerarm
c1=Circle(Coord(-80,-75), 10, layer); --first cloud
c2=Circle(Coord(-125,-115), 20, layer); --second cloud
e=Ellipse(Coord(-250,-210), Coord(-240,0), 70, math.rad(0), math.rad(360), layer) --big cloud
t=Text(Coord(-350,-200),"WHY ME?", 20)

d=active.document()
Builder(d):append(l1):append(c):append(l2):append(l3):append(l4):append(l5):append(l6):append(l7):append(l8):append(l9):append(l10):append(l11):append(c1):append(c2):append(e):execute();
*/

/* LWPolyline

layer = active.proxy.layerByName("0")
vertex2d = { LWVertex2D(Coord(10,10)), LWVertex2D(Coord(10,20)), LWVertex2D(Coord(20,30),-0.5), LWVertex2D(Coord(30,20)), LWVertex2D(Coord(30,10))};
p=LWPolyline(vertex2d,0,0,0,true,Coord(0,0),layer);
d=active.document()
Builder(d):append(p):execute()

 */



/* LWPolyline Closed with start/end width for each entity

layer = active.proxy.layerByName("0")
vertex2d = { LWVertex2D(Coord(10,10),0.25,0,5), LWVertex2D(Coord(10,20),0,5,0), LWVertex2D(Coord(20,30),-0.5), LWVertex2D(Coord(30,20)), LWVertex2D(Coord(30,10))};
p=LWPolyline(vertex2d,0,0,0,true,Coord(0,0),layer);
d=active.document()
Builder(d):append(p):execute()
*/


/* LWPolyline rotate +  scale
layer = active.proxy.layerByName("0")
metaInfo = MetaInfo():add(MetaColor(Color(0,1,0,1)));
vertex2d = { LWVertex2D(Coord(10,10),0.25,0,5), LWVertex2D(Coord(10,20),0,5,0), LWVertex2D(Coord(20,30),-2), LWVertex2D(Coord(30,20)), LWVertex2D(Coord(30,10))};
p=LWPolyline1(vertex2d,0,0,0,true,Coord(0,0),layer, metaInfo);
d=active.document()
Builder(d):append(p):execute()
 */