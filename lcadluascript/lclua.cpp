#include <cad/storage/document.h>
#include "lclua.h"
#include <utils/timer.h>
#include <managers/luacustomentitymanager.h>

using namespace lc::lua;
using namespace LuaIntf;

static const luaL_Reg loadedlibs[] = {
        {"_G", luaopen_base},
        {LUA_LOADLIBNAME, luaopen_package},
        {LUA_COLIBNAME, luaopen_coroutine},
        {LUA_TABLIBNAME, luaopen_table},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        {nullptr, nullptr}
};

LCLua::LCLua(lua_State* L) :
    _L(L),
    _f_openFileDialog(nullptr) {

    LuaBinding(L)
    .beginClass<LuaCustomEntityManager>("LuaCustomEntityManager")
        .addStaticFunction("getInstance", &LuaCustomEntityManager::getInstance)
        .addFunction("registerPlugin", &LuaCustomEntityManager::registerPlugin)
    .endClass();
}

void LCLua::addLuaLibs() {
    const luaL_Reg *lib;

    for (lib = loadedlibs; lib->func != nullptr; lib++) {
        luaL_requiref(_L, lib->name, lib->func, 1);
        lua_pop(_L, 1);
    }

    //Add others non-LC tools
    LuaBinding(_L)
        .addFunction("microtime", &lua_microtime)
        .addFunction("openFile", &openFile)

        .beginClass<FILE>("FILE")
            .addFunction("read", [](FILE* file, const size_t len) {
                return read(file, len);
            })
            .addFunction("write", [](FILE* file, const char* content) {
                return write(file, content);
            })
        .endClass();

    if(_f_openFileDialog == nullptr) {
        LuaBinding(_L).addFunction("openFileDialog", []() {
            return (FILE*) nullptr;
        });
    }
    else {
        LuaBinding(_L).addFunction("openFileDialog", _f_openFileDialog);
    }
}

void LCLua::setDocument(const lc::storage::Document_SPtr& document) {
    LuaIntf::Lua::setGlobal(_L, "document", document);
}

std::string LCLua::runString(const char* code) {
    std::string out;

    auto s = luaL_dostring(_L, code);
    if (s) {
        out.append(lua_tostring(_L, -1));
        lua_pop(_L, 1);
    }

    return out;
}

FILE* LCLua::openFile(const char* path, const char* mode) {
    //TODO: check if the file can be opened

    return fopen(path, mode);
}

std::string LCLua::read(FILE* file, const size_t len) {
    char buf[len + 1];

    size_t n = fread(buf, sizeof(char), len, file);
    buf[n] = '\0';

    return std::string(buf);
}

void LCLua::write(FILE* file, const char* content) {
    fwrite(content, sizeof(char), strlen(content), file);
}

void LCLua::setF_openFileDialog(FILE* (* f_openFileDialog)(bool, const char*, const char*)) {
    _f_openFileDialog = f_openFileDialog;
}
