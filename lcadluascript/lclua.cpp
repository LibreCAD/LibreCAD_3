#include <cad/document/document.h>
#include "lclua.h"
#include <utils/timer.h>

using namespace lc;
using namespace LuaIntf;

static const luaL_Reg loadedlibs[] = {
        {"_G", luaopen_base},
        {LUA_LOADLIBNAME, luaopen_package},
        {LUA_COLIBNAME, luaopen_coroutine},
        {LUA_TABLIBNAME, luaopen_table},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        {LUA_UTF8LIBNAME, luaopen_utf8},
        {NULL, NULL}
};

LCLua::LCLua() {

}

void LCLua::addLuaLibs(lua_State* L) {
    const luaL_Reg *lib;

    for (lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);
    }

    //Add others non-LC functions
    LuaBinding(L)
        .addFunction("microtime", &lua_microtime);
}

void LCLua::setDocument(lua_State* L, lc::Document_SPtr document) {
    LuaIntf::Lua::setGlobal(L, "document", document);
}

std::string LCLua::runString(lua_State* L, const char* code) {
    std::string out;

    auto s = luaL_dostring(L, code);
    if (s != 0) {
        out.append(lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    return out;
}
