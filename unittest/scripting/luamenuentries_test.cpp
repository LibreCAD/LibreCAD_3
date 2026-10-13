// The geometry behind menu entries that used to be connected to nothing.
//
// Each operation keeps the arithmetic that turns what the user clicked into
// an entity in a plain function on its class, so it can be checked here in a
// bare Lua state without the application those operations otherwise need.

#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <lua.hpp>

#include <lclua.h>

#include <cad/meta/layer.h>
#include <cad/operations/layerops.h>
#include <cad/primitive/dimordinate.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>

namespace {

struct LuaMenuEntriesFixture : public ::testing::Test {
    void SetUp() override {
        L = luaL_newstate();
        lcLua = std::make_shared<lc::lua::LCLua>(L);
        lcLua->addLuaLibs();
    }

    void TearDown() override {
        lcLua.reset();
        lua_close(L);
    }

    void loadOperation(const std::string& relativePath) {
        const std::string path = std::string(LCUILUA_SOURCE_DIR) + "/" + relativePath;
        ASSERT_EQ(lcLua->runString(("dofile('" + path + "')").c_str()), "")
            << "could not load " << path;
    }

    lua_State* L{nullptr};
    std::shared_ptr<lc::lua::LCLua> lcLua;
};

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMenuEntriesFixture, FourPointsGiveTheAxisParallelEllipseThroughThem) {
    ASSERT_NO_FATAL_FAILURE(loadOperation("createActions/ellipseoperations.lua"));

    EXPECT_EQ(lcLua->runString(R"LUA(
local through = EllipseOperations.axisParallelEllipseThrough
local function on(cx, cy, a, b, t)
    return {cx + a * math.cos(t), cy + b * math.sin(t)}
end
local function near(a, b) return math.abs(a - b) < 1e-6 * math.max(1, math.abs(b)) end

local x, y, alongX, alongY = through({on(10, 20, 5, 3, 0.3), on(10, 20, 5, 3, 1.9),
                                      on(10, 20, 5, 3, 3.5), on(10, 20, 5, 3, 5.0)})
assert(near(x, 10) and near(y, 20) and near(alongX, 5) and near(alongY, 3), 'a wide ellipse')

x, y, alongX, alongY = through({on(1e5, -2e5, 2, 700, 0.1), on(1e5, -2e5, 2, 700, 2),
                                on(1e5, -2e5, 2, 700, 3), on(1e5, -2e5, 2, 700, 4.4)})
assert(near(x, 1e5) and near(y, -2e5) and near(alongX, 2) and near(alongY, 700),
       'a tall, thin ellipse far from the origin')
)LUA"), "");
}

// Through the corners of a rectangle there is a whole family of such
// ellipses, and through points on a line or on a hyperbola there is none.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMenuEntriesFixture, FourPointsWithNoSingleEllipseGiveNothing) {
    ASSERT_NO_FATAL_FAILURE(loadOperation("createActions/ellipseoperations.lua"));

    EXPECT_EQ(lcLua->runString(R"LUA(
local through = EllipseOperations.axisParallelEllipseThrough
assert(through({{0, 0}, {4, 0}, {4, 2}, {0, 2}}) == nil, 'the corners of a rectangle')
assert(through({{0, 0}, {1, 1}, {2, 2}, {3, 3}}) == nil, 'points on a line')
assert(through({{0, 0}, {1, 0}, {0, 1}, {5, 5}}) == nil, 'points on a hyperbola')
assert(through({{1, 1}, {1, 1}, {1, 1}, {1, 1}}) == nil, 'one point four times')
)LUA"), "");
}

// Arc > Continue: an arc that leaves a point in a given direction and ends
// where the user clicks.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMenuEntriesFixture, AContinuingArcLeavesAlongTheDirectionItIsGiven) {
    ASSERT_NO_FATAL_FAILURE(loadOperation("createActions/arcoperations.lua"));

    EXPECT_EQ(lcLua->runString(R"LUA(
local tangentArc = ArcOperations.tangentArc
local function near(a, b) return math.abs(a - b) < 1e-9 end

local center, radius, startAngle, endAngle, ccw = tangentArc({0, 0}, {2, 0}, {1, 1})
assert(near(center[1], 0) and near(center[2], 1) and near(radius, 1), 'a left turn: the center is on the left')
assert(near(startAngle, -math.pi / 2) and near(endAngle, 0) and ccw, 'a quarter circle, counterclockwise')

center, radius, startAngle, endAngle, ccw = tangentArc({0, 0}, {1, 0}, {1, -1})
assert(near(center[2], -1) and not ccw, 'a right turn runs clockwise')

center, radius, startAngle, endAngle, ccw = tangentArc({0, 0}, {1, 0}, {-1, 1})
assert(near(center[2], 1) and near(radius, 1) and ccw, 'an end point behind gives three quarters of a circle')

assert(tangentArc({0, 0}, {1, 0}, {5, 0}) == nil, 'straight ahead there is no arc')
assert(tangentArc({0, 0}, {1, 0}, {-5, 0}) == nil, 'nor straight back')
assert(tangentArc({0, 0}, {1, 0}, {0, 0}) == nil, 'nor back at the start')
)LUA"), "");
}

// Dimension > ANG2LN: the vertex of the angle between two lines.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMenuEntriesFixture, TwoLinesMeetWhereTheirLinesCross) {
    ASSERT_NO_FATAL_FAILURE(loadOperation("createActions/dimangularoperations.lua"));

    EXPECT_EQ(lcLua->runString(R"LUA(
local cross = DimAngularOperations.lineIntersection
local x, y = cross({0, 0}, {10, 0}, {5, 5}, {5, -3})
assert(x == 5 and y == 0, 'segments that cross')
x, y = cross({0, 0}, {1, 0}, {3, 1}, {3, 2})
assert(x == 3 and y == 0, 'segments that do not reach each other still have lines that cross')
assert(cross({0, 0}, {10, 0}, {0, 1}, {10, 1}) == nil, 'parallel lines do not')
assert(cross({0, 0}, {10, 0}, {20, 0}, {30, 0}) == nil, 'nor do lines along the same line')
)LUA"), "");
}

namespace {

/// A document with the layer "0", handed to the Lua state as `document`, with
/// the kernel bindings the ordinate needs.
std::shared_ptr<lc::storage::DocumentImpl> withDocument(lc::lua::LCLua& lcLua) {
    lcLua.importLCKernel();

    auto document = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    std::make_shared<lc::operation::AddLayer>(
        document, std::make_shared<lc::meta::Layer>("0"))->execute();
    lcLua.setDocument(document);
    return document;
}

}  // namespace

// The Ordinate entry's arithmetic is the one exception to the rule above: it
// lives in the kernel, as DimOrdinateBuilder::dimAuto next to the dimAuto of
// the aligned and radial builders, so that a script in either language places
// an ordinate the way the operation does. This is it, reached from Lua.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMenuEntriesFixture, TheOrdinateLeaderPicksTheAxis) {
    auto document = withDocument(*lcLua);

    EXPECT_EQ(lcLua->runString(R"LUA(
local b = lc.builder.DimOrdinateBuilder()
b:setLayer(document:layerByName('0'))
b:setDefinitionPoint(lc.geo.Coordinate(10, 20))

b:dimAuto(lc.geo.Coordinate(35, 70), lc.geo.Coordinate(36, 90))
local up = b:build()
assert(up:xType(), 'a leader running up measures X')
assert(up:value() == 25, 'the X ordinate is measured from the datum')
assert(up:featurePoint():y() == 70 and up:leaderEndPoint():x() == 36, 'the points did not stick')

b:dimAuto(lc.geo.Coordinate(35, 70), lc.geo.Coordinate(-5, 69))
local aside = b:build()
assert(not aside:xType(), 'a leader running sideways measures Y')
assert(aside:value() == 50, 'the Y ordinate is measured from the datum')

assert(lc.builder.DimOrdinateBuilder.measuresX(lc.geo.Coordinate(0, 0), lc.geo.Coordinate(1, 1)),
       'a leader at 45 degrees measures X')
)LUA"), "");
}

// And the operation behind the entry, driven the way the canvas drives it.
// Only the application it talks to is stubbed; the builder, the entity and
// the document it lands in are the real ones.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMenuEntriesFixture, OrdinateCreatesAnOrdinateDimension) {
    auto document = withDocument(*lcLua);
    ASSERT_NO_FATAL_FAILURE(loadOperation("actions/operations.lua"));
    ASSERT_NO_FATAL_FAILURE(loadOperation("createActions/createOperations.lua"));
    ASSERT_NO_FATAL_FAILURE(loadOperation("createActions/dimordinateoperations.lua"));

    EXPECT_EQ(lcLua->runString(R"LUA(
function message() end
local function nothing() end
luaInterface = {registerEvent = nothing, deleteEvent = nothing, triggerEvent = nothing}

local shown = nil
local temporary = {
    addEntity = function(_, entity) shown = entity end,
    removeEntity = function() shown = nil end,
}
local child = {
    document = function() return document end,
    activeLayer = function() return document:layerByName('0') end,
    metaInfoManager = function() return {metaInfo = nothing} end,
    activeViewport = nothing,
    tempEntities = function() return temporary end,
}
local cli = {returnText = nothing, commandActive = nothing}
mainWindow = {cadMdiChild = function() return child end, cliCommand = function() return cli end}

local op = DimOrdinateOperations()
op:onEvent('mouseMove', {position = lc.geo.Coordinate(3, 4)})
assert(shown == nil, 'a preview before there was a feature to measure')

op:onEvent('point', {position = lc.geo.Coordinate(30, 20)})
op:onEvent('mouseMove', {position = lc.geo.Coordinate(31, 60)})
assert(shown ~= nil and shown:xType(), 'the preview of a leader running up measures X')
op:onEvent('mouseMove', {position = lc.geo.Coordinate(70, 22)})
assert(shown ~= nil and not shown:xType(), 'the preview of a leader running sideways measures Y')

op:onEvent('text', {text = 't'})
op:onEvent('text', {text = 'X <>'})
op:onEvent('point', {position = lc.geo.Coordinate(32, 50)})
assert(op.finished, 'the operation is still running')
assert(shown == nil, 'the preview was left behind')
)LUA"), "");

    std::vector<lc::entity::DimOrdinate_CSPtr> ordinates;
    for (const auto& entity : document->entityContainer().asVector()) {
        if (auto ordinate = std::dynamic_pointer_cast<const lc::entity::DimOrdinate>(entity)) {
            ordinates.push_back(ordinate);
        }
    }
    ASSERT_EQ(ordinates.size(), 1u) << "the operation did not add one ordinate dimension";

    const auto& ordinate = ordinates.front();
    EXPECT_TRUE(ordinate->xType());
    EXPECT_DOUBLE_EQ(ordinate->value(), 30.0);
    EXPECT_EQ(ordinate->explicitValue(), "X <>");
    EXPECT_DOUBLE_EQ(ordinate->definitionPoint().x(), 0.0);
    EXPECT_DOUBLE_EQ(ordinate->definitionPoint().y(), 0.0);
    EXPECT_DOUBLE_EQ(ordinate->featurePoint().x(), 30.0);
    EXPECT_DOUBLE_EQ(ordinate->featurePoint().y(), 20.0);
    EXPECT_DOUBLE_EQ(ordinate->leaderEndPoint().x(), 32.0);
    EXPECT_DOUBLE_EQ(ordinate->leaderEndPoint().y(), 50.0);
    EXPECT_DOUBLE_EQ(ordinate->middleOfText().y(), 50.0);
    ASSERT_NE(ordinate->layer(), nullptr);
    EXPECT_EQ(ordinate->layer()->name(), "0");
}
