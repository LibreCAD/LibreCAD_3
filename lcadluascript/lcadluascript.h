#pragma once

#include <cad/document/document.h>
#include <cad/document/storagemanager.h>


class LCadLuaScript {

    public:
        LCadLuaScript(lc::Document* document, lc::StorageManager_SPtr storageManager);
        LCadLuaScript(lc::Document* document, lc::StorageManager_SPtr storageManager, bool usePrintLib);
        std::string run(const std::string& script);
    private:
        lc::Document* _document;
        lc::StorageManager_SPtr _storageManager;
        bool _usePrintLib;
};
