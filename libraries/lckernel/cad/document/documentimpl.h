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

    class DocumentImpl : public virtual AbstractDocument {
            Q_OBJECT
        public:
            DocumentImpl(LayerManager* layerManager, EntityManager* entityManager);
            virtual ~DocumentImpl();


            void operateOn(Operation* operation);

        protected:
            virtual void begin(Operation* operation);
            virtual void commit(Operation* operation);

        public:
        signals:
            void beginProcessEvent(lc::BeginProcessEvent* event);
            void commitProcessEvent(lc::CommitProcessEvent* event);

            // Etity events
            void addEntityEvent(lc::AddEntityEvent* event);
            void replaceEntityEvent(lc::ReplaceEntityEvent* event);
            void removeEntityEvent(lc::RemoveEntityEvent* event);

        public:
            virtual void addEntity(const QString& layerName, CADEntity* cadEntity);
            virtual void replaceEntity(CADEntity* oldEntity, CADEntity* newEntity);
            virtual void removeEntity(ID_DATATYPE id);

            virtual LayerManager* layerManager() const;

            virtual CADEntity* findEntityByID(ID_DATATYPE id) const;
            virtual QString findEntityLayerByID(ID_DATATYPE id) const;
        private:
            virtual void lock();
            virtual void releaseLock() ;
        private:
            LayerManager* _layerManager;
            bool _locked;

            // We need to have a betetr way of objects that need to be deleted from memory, and objects that needs to stay in memory
            // So this is a FIXME PLEASE
            // In general object lifetime is simple
            // RULE: The vectors below is a place where we are storing ALL objects that are active in the document, that includes
            // everything in undo stage

            // We add everything to the vector during a addXXXX (..) operation
            // When the document is closed we go through all entities in the below vectors and delete them in the destructor of the DocumentImpl
            // When a operation get0's removed from the undo stack we ask the operation what entities where deleted and we then delete them from the below list and from memory
            // Note Dongxu suggests to use boost smart pointers might relieve us from the memory mangement
            QVector<CADEntity*> _allEntites;
            QVector<void*> _allFonts;
            QVector<void*> _AllBlocks;
            QVector<void*> _allLayers;

    };
}

#endif


