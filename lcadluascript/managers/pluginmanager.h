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

            /**
             * @brief Load the Lua plugins
             * @param f_openFileDialog Function which ask the user to choose a file, this might be moved later
             */
            void loadPlugins(FILE* (*f_openFileDialog)(bool, const char*, const char*));

        private:
            void loadPlugin(const char* file, FILE* (*f_openFileDialog)(bool, const char*, const char*));

            const char* _interface;
    };
}