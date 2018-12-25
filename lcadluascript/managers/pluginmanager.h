#pragma once

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <map>

namespace lc {
    namespace lua {
        class PluginManager {
            public:
                PluginManager(lua_State* l, const char* interface);

                /**
                 * @brief Load the Lua plugins
                 */
                void loadPlugins();

            private:
                void loadPlugin(const char* file);

                lua_State* _L;
                const char* _interface;
        };
    }
}