#pragma once

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <map>
#include <string>

namespace lc {
namespace lua {
class PluginManager {
public:
    PluginManager(lua_State* l, const char* iface);

    /**
     * @brief Load the Lua plugins from the configured plugin directory.
     *
     * Phase 5 PR-5.5: previously hardcoded `../lcUILua/plugins/` at
     * the call site; now reads path.lua's `plugin_path` global at
     * initLua time (the Lua-facing lookup already happened before
     * PluginManager runs, so the state has it) and delegates to
     * `loadPluginsFrom(path)`.
     */
    void loadPlugins();

    /**
     * @brief Load the Lua plugins from an explicit directory.  Public
     * so callers with alternate paths (unittest fixtures, phase-6
     * multi-window scenarios) can override.
     */
    void loadPluginsFrom(const std::string& path);

private:
    void loadPlugin(const char* file);

    lua_State* _L;
    const char* _interface;
};
}
}