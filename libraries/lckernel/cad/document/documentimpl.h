#ifndef DOCUMENTIMPL_H
#define DOCUMENTIMPL_H

#include <QObject>
#include <QVector>
#include "cad/document/document.h"
#include "cad/document/layermanager.h"
#include "cad/document/selectionmanager.h"
#include "cad/operations/operation.h"

namespace lc {

    class DocumentImpl : public Document {
            Q_OBJECT
        public:
            DocumentImpl(LayerManager* layerManager);
            virtual ~DocumentImpl();

            virtual LayerManager* layerManager() const;


        private:
            virtual void addEntity(CADEntity* cadEntity) const;
            virtual void replaceEntity(CADEntity* oldEntity, CADEntity* newEntity) const;
            virtual void removeEntity(ID_DATATYPE id) const;

            virtual void lock();
            virtual void releaseLock() ;
        private:
            LayerManager* _layerManager;
            bool _locked;
    };
}

#endif


