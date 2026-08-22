#include "pluginmanager.h"
#include "lclua.h"
#include <dirent.h>
#include <iostream>
#include <kaguya/kaguya.hpp>

using namespace lc::lua;

PluginManager::PluginManager(lua_State* l, const char* interface) :
    _L(l),
    _interface(interface) {

}

void PluginManager::loadPlugins() {
    // Phase 5 PR-5.5 — was `const char* path = "../lcUILua/plugins/";`
    // hardcoded relative-to-CWD path that only worked when librecad was
    // launched from the build directory.  Now reads `plugin_path` from
    // Lua state (set by path.lua at initLua time — see
    // LuaInterface::initLua's dofile call).
    kaguya::State state(_L);
    kaguya::LuaRef pluginPathRef = state["plugin_path"];
    if (pluginPathRef.isNilref()) {
        // path.lua never ran or the global was cleared.  Silent no-op
        // matches the pre-refactor "opendir failed" branch which
        // perror'd and continued.
        return;
    }

    std::string path = pluginPathRef.get<std::string>();
    if (path.empty()) return;
    // Ensure trailing slash so `path + entry + "/plugin.lua"` composes
    // correctly.
    if (path.back() != '/' && path.back() != '\\') {
        path.push_back('/');
    }
    loadPluginsFrom(path);
}

void PluginManager::loadPluginsFrom(const std::string& path) {
    DIR* dir;
    struct dirent* ent;

    if ((dir = opendir(path.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            loadPlugin((path + ent->d_name + "/plugin.lua").c_str());
        }
        closedir(dir);
    }
    else {
        // opendir failed — the plugin directory doesn't exist.  Not a
        // fatal error (fresh install may have no plugins yet); log
        // but don't abort init.
        // (Retained perror parity with the pre-refactor behavior; a
        // future cleanup can route through a structured logger.)
        perror(("Plugin directory " + path + " opendir").c_str());
    }
}

void PluginManager::loadPlugin(const char* file) {
    kaguya::State state(_L);
    state["LC_interface"] = _interface;
    bool s = state.dofile(file);

    // Phase 5 PR-5.5 fixup — kaguya::State::dofile returns TRUE on
    // SUCCESS, verified via third_party/kaguya/include/kaguya/state.hpp
    // (dofile's own doc comment).  The PR-5.5 "fix" got the polarity
    // exactly backwards: `if (!s) return; else print error` printed
    // a bogus "Plugin X load failed: <stack top>" on every SUCCESSFUL
    // plugin load AND swallowed real errors silently.  Correct
    // behavior: success returns silently; failure prints the error
    // string from Lua's stack.
    //
    // Note on luainterface.cpp:40's identically-shaped `if (s)` block
    // (Lua's output-tostring path) — verified via `git log --grep`
    // it has NEVER been touched despite the earlier comment claiming
    // phase-4 fixed it; that path is log-only and out of scope for
    // this PR (pre-existing, tracked separately if worth surfacing).
    if (s) {
        // Load ok — nothing to print.
        return;
    }
    const char* err = lua_tostring(_L, -1);
    if (err != nullptr) {
        std::cerr << "Plugin " << file << " load failed: "
                  << err << std::endl;
    }
    lua_pop(state.state(), 1);
}
