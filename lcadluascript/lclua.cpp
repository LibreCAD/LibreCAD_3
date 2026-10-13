#include <cad/storage/document.h>
#include "lclua.h"
#include <utils/timer.h>
#include <managers/luacustomentitymanager.h>
#include <scriptadapter/customentitydispatch_lua.h>  // Phase 6 PR-6.1 sub-piece 2b
#include <kaguya/kaguya.hpp>
#include <bridge/lc.h>
#include <bridge/lc_geo.h>
#include <bridge/lc_meta.h>
#include <bridge/lc_entity.h>
#include <bridge/lc_builder.h>
#include <bridge/lc_storage.h>
#include <bridge/lc_maths.h>
#include <bridge/lc_operation.h>
#include <bridge/lc_event.h>
#if USE_PERSISTENCE
#include <bridge/lc_persistence.h>
#endif

using namespace lc::lua;

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

    kaguya::State s(_L);
    s["registerPlugin"].setFunction([](const std::string& name, kaguya::LuaRef onNewWaitingEntityFunction) {
        LuaCustomEntityManager::getInstance().registerPlugin(name, onNewWaitingEntityFunction);
    });

    // Phase 6 PR-6.1 sub-piece 2b — install the CustomEntity dispatch
    // hook so ScriptCustomEntity's 6 dispatch methods (which live in
    // lcscripting post-move) can reach the Lua fast path via
    // unwrapLuaCallback + kaguya's static template deduction.
    // Idempotent — safe to call once per LuaInterface / LCLua
    // construction; the process-global slot always points at a fresh
    // hook instance (which is stateless).
    lc::lua::installLuaCustomEntityDispatchHook();
}

void LCLua::addLuaLibs() {
    const luaL_Reg *lib;

    for (lib = loadedlibs; lib->func != nullptr; lib++) {
        luaL_requiref(_L, lib->name, lib->func, 1);
        lua_pop(_L, 1);
    }

    //Add others non-LC tools
    kaguya::State s(_L);
    s["microtime"].setFunction(&lua_microtime);
    s["openFile"].setFunction(&openFile);

    s["FILE"].setClass(kaguya::UserdataMetatable<FILE>()
    .addStaticFunction("read", [](FILE* file, const size_t len) {
        return read(file, len);
    })
    .addStaticFunction("write", [](FILE* file, const char* content) {
        return write(file, content);
    })
                      );

    if(_f_openFileDialog == nullptr) {
        s["openFileDialog"].setFunction([]() {
            return (FILE*) nullptr;
        });
    }
    else {
        s["openFileDialog"].setFunction(_f_openFileDialog);
    }
}

void LCLua::setDocument(const lc::storage::Document_SPtr& document) {
    kaguya::State state(_L);
    state["document"] = document;
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

std::string LCLua::read(FILE* file, size_t len) {
    char* buf = new char[len + 1];

    size_t n = fread(buf, sizeof(char), len, file);
    buf[n] = '\0';

    auto bufferStr = std::string(buf);
    delete[] buf;   // allocated with new[]

    return bufferStr;
}

void LCLua::write(FILE* file, const char* content) {
    fwrite(content, sizeof(char), strlen(content), file);
}

void LCLua::setF_openFileDialog(FILE* (* f_openFileDialog)(bool, const char*, const char*)) {
    _f_openFileDialog = f_openFileDialog;
}

void LCLua::importLCKernel() {
    kaguya::State state(_L);

    import_lc_namespace(state);
    import_lc_geo_namespace(state);
    import_lc_meta_namespace(state);
    import_lc_entity_namespace(state);
    import_lc_builder_namespace(state);
    import_lc_storage_namespace(state);
    import_lc_maths_namespace(state);
    import_lc_event_namespace(state);
    import_lc_operation_namespace(state);
#if USE_PERSISTENCE
    // Phase 2 slice 2.6 — persistence bindings, in parity with the Python
    // side.  Gated on USE_PERSISTENCE so an OFF build keeps working.
    import_lc_persistence_namespace(state);
#endif
}
