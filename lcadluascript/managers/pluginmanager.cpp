#include "pluginmanager.h"
#include "lclua.h"
#include <dirent.h>

lc::PluginManager::PluginManager(lua_State* l, const char* interface) :
    _L(l),
    _interface(interface) {

}

void lc::PluginManager::loadPlugins() {
    const char* path = "../lcUILua/plugins/"; //TODO: get path

    DIR* dir;
    struct dirent* ent;

    if((dir = opendir(path)) != nullptr) {
        while((ent = readdir(dir)) != nullptr) {
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
    auto state = LuaIntf::LuaState(_L);
    LuaIntf::Lua::setGlobal(state, "LC_interface", _interface);
    bool s = state.doFile(file);

    if (s) {
        std::cout << lua_tostring(_L, -1) << std::endl;
        lua_pop(state, 1);
    }
}