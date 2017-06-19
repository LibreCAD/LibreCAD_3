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
            LCLua(lua_State* L);

            void addLuaLibs();
            void importLCKernel();
            void setDocument(lc::Document_SPtr document);
            std::string runString(const char* code);

            static FILE* openFile(const char* path, const char* mode);
            static std::string read(FILE* file, const size_t len);
            static void write(FILE* file, const char* content);

        private:
            lua_State* _L;
    };
}