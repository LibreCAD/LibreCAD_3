#ifndef DOCUMENTIMPL_H
#define DOCUMENTIMPL_H

#include <QObject>
#include <QVector>
#include "cad/const.h"

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
#include "cad/events/absoleteentityevent.h"

namespace lc {

    class DocumentImpl : public AbstractDocument {
            Q_OBJECT
        public:
            DocumentImpl();
            virtual ~DocumentImpl();

            void operateOn(shared_ptr<lc::Operation> operation);
            shared_ptr<lc::LayerManager> layerManager() const;
            void setLayerManager(shared_ptr<lc::LayerManager> layerManager);

        public:
        signals:
            void beginProcessEvent(const lc::BeginProcessEvent&);
            void commitProcessEvent(const lc::CommitProcessEvent&);

            // Entity events
            void addEntityEvent(const lc::AddEntityEvent&);
            void replaceEntityEvent(const lc::ReplaceEntityEvent&);
            void removeEntityEvent(const lc::RemoveEntityEvent&);
            void absoleteEntityEvent(const lc::AbsoluteEntityEvent&);

        protected:
            virtual void begin(shared_ptr<lc::Operation> operation);
            virtual void commit(shared_ptr<lc::Operation> operation);
        public:
            virtual void addEntity(const QString& layerName, shared_ptr<const lc::CADEntity> cadEntity);
            virtual void replaceEntity(shared_ptr<const lc::CADEntity> oldEntity, shared_ptr<const lc::CADEntity> newEntity);
            virtual void removeEntity(ID_DATATYPE id);
            virtual void absoleteEntity(shared_ptr<const lc::CADEntity> entity);

            virtual shared_ptr<const lc::CADEntity> findEntityByID(ID_DATATYPE id) const;
            virtual QString findEntityLayerByID(ID_DATATYPE id) const;
        private:
            virtual void lock();
            virtual void releaseLock() ;
        private:
            shared_ptr<lc::LayerManager> _layerManager;
            bool _locked;

    };
}

#endif


