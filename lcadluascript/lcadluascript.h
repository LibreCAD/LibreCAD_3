#pragma once

#include <cad/document/document.h>
#include <cad/document/storagemanager.h>



class LCadLuaScript {

    public:
        LCadLuaScript(std::shared_ptr<lc::Document> document);
        LCadLuaScript(std::shared_ptr<lc::Document>, bool usePrintLib);
        std::string run(const std::string& script);
    private:
        std::shared_ptr<lc::Document> _document;
        bool _usePrintLib;
};
