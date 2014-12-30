#pragma once

#include <cad/document/document.h>
#include <cad/document/storagemanager.h>



class LCadLuaScript {

    public:
        LCadLuaScript(lc::Document* document);
        LCadLuaScript(lc::Document* document, bool usePrintLib);
        std::string run(const std::string& script);
    private:
        lc::Document* _document;
        bool _usePrintLib;
};
