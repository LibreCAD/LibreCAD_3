#include "pluginmanager.h"
#include "lclua.h"
#include <dirent.h>
#include <kaguya/kaguya.hpp>

using namespace lc::lua;

PluginManager::PluginManager(lua_State* l, const char* interface) :
    _L(l),
    _interface(interface) {

}

void PluginManager::loadPlugins() {
    const char* path = "../finalInstallDir/usr/share/librecad/lcUILua/plugins/";

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

void PluginManager::loadPlugin(const char* file) {
    kaguya::State state(_L);
    state["LC_interface"] = _interface;
    bool s = state.dofile(file);

    if (s) {
        std::cout << lua_tostring(_L, -1) << std::endl;
        lua_pop(state.state(), 1);
    }
}
