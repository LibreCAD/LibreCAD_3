#pragma once

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

#include "lua-intf/LuaIntf/LuaIntf.h"
#include <cad/document/document.h>

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
    LUA_USING_LIST_TYPE(std::vector)
}

namespace lc {
    class LCLua {
        public:
            LCLua();

            static void addLuaLibs(lua_State* L);
            static void importLCKernel(lua_State* L);
            static void setDocument(lua_State* L, lc::Document_SPtr document);
            static std::string runString(lua_State* L, const char* code);

            static std::string openFile(std::string path);
            //TODO: 1 global version with path as param
            //TODO: 1 local version with dialog -> subclass LCLua
    };
}