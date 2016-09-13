#pragma once

#include "const.h"
#include <cad/document/document.h>
#include <cad/document/storagemanager.h>


class LCadLuaScript {

    public:
        DLL_EXPORT LCadLuaScript(std::shared_ptr<lc::Document> document);
        DLL_EXPORT LCadLuaScript(std::shared_ptr<lc::Document>, bool usePrintLib);
        std::string DLL_EXPORT run(const std::string& script);
    private:
        std::shared_ptr<lc::Document> _document;
        bool _usePrintLib;
};
