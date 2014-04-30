#ifndef LCADLUASCRIPT_H
#define LCADLUASCRIPT_H

#include "lcadluascript_global.h"

#include <cad/document/document.h>
#include <cad/document/storagemanager.h>


class LCADLUASCRIPTSHARED_EXPORT LCadLuaScript {

    public:
        LCadLuaScript(lc::Document* document, std::shared_ptr<lc::StorageManager> storageManager);
        QString run(const QString& script);
    private:
        lc::Document* _document;
        std::shared_ptr<lc::StorageManager> _storageManager;
};

#endif // LCADLUASCRIPT_H
