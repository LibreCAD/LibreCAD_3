#ifndef DOCUMENTIMPL_H
#define DOCUMENTIMPL_H

#include <QObject>
#include <QVector>
#include "cad/document/document.h"
#include "cad/document/layermanager.h"
#include "cad/document/selectionmanager.h"

namespace lc {

    class DocumentImpl : public Document {
            Q_OBJECT
        public:
            DocumentImpl(LayerManager* layerManager);
            virtual ~DocumentImpl();

            virtual LayerManager* layerManager() const;

        private:
            LayerManager* _layerManager;
    };
}

#endif


