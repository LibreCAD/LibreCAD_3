#pragma once

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "lua-intf/LuaIntf/LuaIntf.h"

namespace lc {
    class PluginManager {
        public:
            PluginManager(const char* interface);
            void loadPlugins();

        private:
            void loadPlugin(const char* file);

            const char* _interface;
    };
}