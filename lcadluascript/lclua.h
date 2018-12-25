#pragma once

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

#include <cad/storage/document.h>

namespace lc {
    namespace lua {
        class LCLua {
            public:
                LCLua(lua_State* L);

                /**
                 * @brief Bind Lua functions and others functions required by LibreCAD
                 * @param f_openFileDialog Function which asks user to choose a file
                 */
                void addLuaLibs();

                void importLCKernel();

                void setDocument(const lc::storage::Document_SPtr& document);

                std::string runString(const char* code);

                void setF_openFileDialog(FILE* (* f_openFileDialog)(bool, const char*, const char*));

                static FILE* openFile(const char* path, const char* mode);

                static std::string read(FILE* file, const size_t len);

                static void write(FILE* file, const char* content);

            private:
                lua_State* _L;

                FILE* (* _f_openFileDialog)(bool, const char*, const char*);
        };
    }
}