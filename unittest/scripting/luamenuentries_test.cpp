// The geometry behind menu entries that used to be connected to nothing.
//
// Each operation keeps the arithmetic that turns what the user clicked into
// an entity in a plain function on its class, so it can be checked here in a
// bare Lua state without the application those operations otherwise need.

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <lua.hpp>

#include <lclua.h>

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
