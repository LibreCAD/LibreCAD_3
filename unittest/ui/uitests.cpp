#include "uitests.h"
#include <gtest/gtest.h>
#include <lclua.h>

TEST(LuaRuntimeTest, UiDllAndExecutableShareLuaCore) {
    // The constructor creates the state inside lcui, not this executable.
    // Lua 5.3 compares these core-identity pointers in luaL_checkversion;
    // separate static Lua copies must fail here before a later API panic.
    // Lua 5.4+ returns a version value instead, retaining the ABI check.
    lc::ui::LuaInterface luaInterface;
    ASSERT_EQ(lua_version(nullptr), lua_version(luaInterface.luaState()));
    lc::lua::LCLua lua(luaInterface.luaState());
    EXPECT_EQ("", lua.runString("assert(type(registerPlugin) == 'function')"));
}

storage::Document_SPtr createDocument() {
    auto storageManager = std::make_shared<storage::StorageManagerImpl>();
    return std::make_shared<storage::DocumentImpl>(storageManager);
};
