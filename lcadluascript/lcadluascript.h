#ifndef LCADLUASCRIPT_H
#define LCADLUASCRIPT_H

#include "lcadluascript_global.h"

#include <cad/document/document.h>
#include <cad/document/storagemanager.h>


class LCADLUASCRIPTSHARED_EXPORT LCadLuaScript {

    public:
        LCadLuaScript(lc::Document* document, lc::StorageManager_SPtr storageManager);
        QString run(const QString& script);
    private:
        lc::Document* _document;
        lc::StorageManager_SPtr _storageManager;
};

#endif // LCADLUASCRIPT_H
