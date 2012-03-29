#ifndef DOCUMENTIMPL_H
#define DOCUMENTIMPL_H

#include <QObject>
#include <QVector>
#include "cad/document/abstractdocument.h"
#include "cad/document/entitymanager.h"
#include "cad/document/layermanager.h"
#include "cad/document/selectionmanager.h"
#include "cad/operations/operation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

namespace lc {

    class AbstractDocumentImpl : public virtual AbstractDocument {
            Q_OBJECT
        public:
            AbstractDocumentImpl(LayerManager* layerManager, EntityManager* entityManager);
            virtual ~AbstractDocumentImpl();


            void operateOn(Operation* operation);

        protected:
            virtual void begin(Operation* operation);
            virtual void commit(Operation* operation);

        public:
        signals:
            void beginProcessEvent(BeginProcessEvent* event);
            void commitProcessEvent(CommitProcessEvent* event);
            void addEntityEvent(lc::AddEntityEvent* event);
            void replaceEntityEvent(ReplaceEntityEvent* event);
            void removeEntityEvent(RemoveEntityEvent* event);

        public:
            virtual void addEntity(const QString& layerName, CADEntity* cadEntity);
            virtual void replaceEntity(CADEntity* oldEntity, CADEntity* newEntity);
            virtual void removeEntity(ID_DATATYPE id);

            virtual LayerManager* layerManager() const;
        private:
            virtual void lock();
            virtual void releaseLock() ;
        private:
            LayerManager* _layerManager;
            bool _locked;
    };
}

#endif


