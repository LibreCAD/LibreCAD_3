#ifndef DOCUMENTIMPL_H
#define DOCUMENTIMPL_H

#include <QObject>
#include <QVector>
#include "cad/document/document.h"
#include "cad/document/layermanager.h"
#include "cad/document/selectionmanager.h"
#include "cad/operations/operation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

namespace lc {

    class DocumentImpl : public virtual Document {
            Q_OBJECT
        public:
            DocumentImpl(LayerManager* layerManager);
            virtual ~DocumentImpl();

            virtual LayerManager* layerManager() const;

            void operateOn(Operation* operation);

        protected:
            virtual void begin(Operation* operation);
            virtual void commit(Operation* operation);

        public:
        signals:
             void beginProcessEvent(BeginProcessEvent * event);
             void commitProcessEvent(CommitProcessEvent * event);

            /**
            * All operations that are possible on entities
            */
             void addEntityEvent(lc::AddEntityEvent * event);
             void replaceEntityEvent(ReplaceEntityEvent * event);
             void removeEntityEvent(RemoveEntityEvent * event);

        public:
            virtual void addEntity(CADEntity* cadEntity);
            virtual void replaceEntity(CADEntity* oldEntity, CADEntity* newEntity);
            virtual void removeEntity(ID_DATATYPE id);

        private:
            virtual void lock();
            virtual void releaseLock() ;
        private:
            LayerManager* _layerManager;
            bool _locked;
    };
}

#endif


