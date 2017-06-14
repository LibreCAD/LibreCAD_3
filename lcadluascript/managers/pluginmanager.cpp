#include "pluginmanager.h"
#include <dirent.h>
#include <iostream>
#include <cad/lualibrecadbridge.h>

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

lc::PluginManager::PluginManager(const char* interface) :
    _interface(interface) {

}

void lc::PluginManager::loadPlugins() {
    const char* path = "../lcUILua/plugins/"; //TODO: get path

    DIR* dir;
    struct dirent* ent;

    if((dir = opendir(path)) != NULL) {
        while((ent = readdir(dir)) != NULL) {
            if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            loadPlugin((std::string(path) + ent->d_name + "/plugin.lua").c_str());
        }
        closedir(dir);
    }
    else {
        perror("Error opening dir");
    }
}

void lc::PluginManager::loadPlugin(const char* file) {
    auto state = LuaIntf::LuaState::newState();

    addLuaLibs(state);
    lua_openlckernel(state);

    LuaIntf::Lua::setGlobal(state, "LC_interface", _interface);

    int s = state.doFile(file);

    if (s != 0) {
        std::cout << lua_tostring(state, -1) << std::endl;
        lua_pop(state, 1);
    }
}

void lc::PluginManager::addLuaLibs(lua_State* L) {
    const luaL_Reg *lib;

    for (lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);
    }
}
