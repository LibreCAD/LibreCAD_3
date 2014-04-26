#ifndef LCADLUASCRIPT_H
#define LCADLUASCRIPT_H

#include "lcadluascript_global.h"

#include <cad/document/document.h>
#include <cad/document/entitymanager.h>


class LCADLUASCRIPTSHARED_EXPORT LCadLuaScript {

    public:
        LCadLuaScript(lc::Document* document, shared_ptr<lc::EntityManager> entityManager, shared_ptr<lc::LayerManager> layerManager);
        QString run(const QString& script);
    private:
        lc::Document* _document;
        shared_ptr<lc::EntityManager> _entityManager;
        shared_ptr<lc::LayerManager> _layerManager;
};

#endif // LCADLUASCRIPT_H
