// The text builders, from Lua.
//
// Nothing anywhere exercised a Text or MText builder, in either scripting
// language, so a binding that was never added looked exactly like one that
// worked. Most of them had never been added: a script could set a text's
// position, content, height and angle and had no way to say how it was
// aligned, or to make an MText bold.
//
// Needs lcluascript, kaguya and a real Lua runtime, so it sits with the other
// kaguya-backed tests.

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <lua.hpp>

#include <lclua.h>

#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/meta/layer.h>
#include <cad/operations/layerops.h>
#include <cad/primitive/line.h>

namespace {

struct LuaTextBuilderFixture : public ::testing::Test {
    void SetUp() override {
        L = luaL_newstate();
        lcLua = std::make_shared<lc::lua::LCLua>(L);
        lcLua->addLuaLibs();
        lcLua->importLCKernel();

        document = std::make_shared<lc::storage::DocumentImpl>(
            std::make_shared<lc::storage::StorageManagerImpl>());
        lcLua->setDocument(document);

        // A layer the document owns. The real operations take
        // mainWindow:cadMdiChild():activeLayer(); a freshly constructed
        // lc.meta.Layer is not a Layer_CSPtr and setLayer keeps its null.
        auto layer = std::make_shared<lc::meta::Layer>("0");
        std::make_shared<lc::operation::AddLayer>(document, layer)->execute();
    }

    void TearDown() override {
        lcLua.reset();
        lua_close(L);
    }

    lua_State* L{nullptr};
    std::shared_ptr<lc::lua::LCLua> lcLua;
    std::shared_ptr<lc::storage::DocumentImpl> document;
};

}  // namespace

// Without these a script can call setHorizontalAlign and has no name for what
// to pass.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaTextBuilderFixture, AlignmentConstantsAreNamed) {
    EXPECT_EQ(lcLua->runString(R"LUA(
assert(lc.TextConst ~= nil, 'lc.TextConst is missing')
assert(lc.TextConst.HALeft == 0, 'HALeft')
assert(lc.TextConst.HACenter == 1, 'HACenter')
assert(lc.TextConst.HARight == 2, 'HARight')
assert(lc.TextConst.VABaseline == 0, 'VABaseline')
assert(lc.TextConst.VABottom == 1, 'VABottom')
assert(lc.TextConst.VAMiddle == 2, 'VAMiddle')
assert(lc.TextConst.VATop == 3, 'VATop')
assert(lc.TextConst.None == 0, 'None')
)LUA"), "");
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaTextBuilderFixture, ATextCarriesItsAlignmentThroughTheBuilder) {
    EXPECT_EQ(lcLua->runString(R"LUA(
local b = lc.builder.TextBuilder()
b:setInsertionPoint(lc.geo.Coordinate(1, 2))
b:setTextValue('hello')
b:setHeight(2.5)
-- A layer the document owns, which is what the real operations use
-- (mainWindow:cadMdiChild():activeLayer()). A freshly constructed one is
-- not a Layer_CSPtr and setLayer silently keeps its null.
b:setLayer(document:layerByName('0'))
b:setHorizontalAlign(lc.TextConst.HACenter)
b:setVerticalAlign(lc.TextConst.VATop)
b:setTextFont('STANDARD')

assert(b:horizontalAlign() == lc.TextConst.HACenter, 'halign did not stick')
assert(b:verticalAlign() == lc.TextConst.VATop, 'valign did not stick')
assert(b:textStyle() == 'STANDARD', 'style did not stick')

local t = b:build()
assert(t ~= nil, 'build returned nil')
assert(t:halign() == lc.TextConst.HACenter, 'the entity lost the halign')
assert(t:valign() == lc.TextConst.VATop, 'the entity lost the valign')
)LUA"), "");
}

// The four flags that make an MText an MText. They round-trip through the
// builder and the property editor and were reachable from neither language.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaTextBuilderFixture, AnMTextCarriesItsStyleFlags) {
    EXPECT_EQ(lcLua->runString(R"LUA(
local b = lc.builder.MTextBuilder()
b:setInsertionPoint(lc.geo.Coordinate(0, 0))
b:setTextValue('two\nlines')
b:setHeight(1.0)
-- A layer the document owns, which is what the real operations use
-- (mainWindow:cadMdiChild():activeLayer()). A freshly constructed one is
-- not a Layer_CSPtr and setLayer silently keeps its null.
b:setLayer(document:layerByName('0'))
b:setBold(true)
b:setItalic(true)
b:setUnderlined(true)
b:setStrikethrough(false)

assert(b:bold() == true, 'bold did not stick')
assert(b:italic() == true, 'italic did not stick')
assert(b:underlined() == true, 'underlined did not stick')
assert(b:strikethrough() == false, 'strikethrough did not stick')

local m = b:build()
assert(m ~= nil, 'build returned nil')
assert(m:bold() == true, 'the entity lost bold')
assert(m:italic() == true, 'the entity lost italic')
assert(m:underlined() == true, 'the entity lost underlined')
assert(m:strikethrough() == false, 'the entity lost strikethrough')
assert(m:text_value() == 'two\nlines', 'the entity lost the newline')
)LUA"), "");
}

TEST_F(LuaTextBuilderFixture, LockedLayerOperationReportsRefusalAndAllowsExplicitOverride) {
    auto layer = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1), lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(layer);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), layer, nullptr);
    document->insertEntity(line);
    const auto err = lcLua->runString(R"LUA(
local entities = document:entityContainer():asVector(32767)
local moved = entities[1]:move(lc.geo.Coordinate(2, 0))
local blocked = lc.operation.EntityBuilder.new(document)
blocked:appendEntity(moved)
blocked:execute()
assert(blocked:blockedByLockedLayer())
local force = lc.operation.EntityBuilder.new(document)
force:setAllowLockedLayerEdits(true)
force:appendEntity(moved)
force:execute()
assert(not force:blockedByLockedLayer())
local later = moved:move(lc.geo.Coordinate(3, 0))
assert(not document:tryInsertEntity(later))
assert(not document:tryRemoveEntity(moved))
document:insertEntity(later)
document:removeEntity(moved)
assert(#document:entityContainer():asVector(32767) == 1)
document:forceInsertEntity(later)
document:forceRemoveEntity(later)
assert(#document:entityContainer():asVector(32767) == 0)
assert(document:tryInsertEntity(moved)) -- creation on a locked layer remains allowed
document:forceInsertEntity(nil)
document:forceRemoveEntity(nil)
assert(not pcall(function() lc.storage.DocumentImpl.create(nil) end))
assert(not pcall(function() lc.storage.DocumentImpl(lc.storage.StorageManagerImpl()) end))
local other = lc.storage.DocumentImpl.create(lc.storage.StorageManagerImpl.create())
assert(other:tryInsertEntity(moved))
assert(not other:tryRemoveEntity(moved))
other:forceRemoveEntity(moved)
assert(#other:entityContainer():asVector(32767) == 0)
)LUA");
    ASSERT_EQ(err, "") << err;
    const auto moved = std::dynamic_pointer_cast<const lc::entity::Line>(document->entityByID(line->id()));
    ASSERT_TRUE(moved);
    EXPECT_DOUBLE_EQ(moved->start().x(), 2.0);
}
