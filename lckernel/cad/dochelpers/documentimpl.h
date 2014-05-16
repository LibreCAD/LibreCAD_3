#ifndef DOCUMENTIMPL_H
#define DOCUMENTIMPL_H

#include <QObject>
#include <QVector>
#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include <cad/events/addlayerevent.h>
#include <cad/events/removelayerevent.h>
#include <cad/events/replacelayerevent.h>
#include "cad/const.h"

#include "cad/document/document.h"
#include "cad/document/storagemanager.h"
#include "cad/document/selectionmanager.h"
#include "cad/operations/documentoperation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

namespace lc {

    class DocumentImpl : public Document {
            Q_OBJECT
        public:
            DocumentImpl(const StorageManager_SPtr storageManager);
            virtual ~DocumentImpl();

        public:
        signals:
            /*!
             * \brief begins a Process Event
             */
            void beginProcessEvent(const lc::BeginProcessEvent&);
            /*!
             * \brief commits a Process Event
             */
            void commitProcessEvent(const lc::CommitProcessEvent&);

            /*!
             * \brief Event to add an Entity
             */
            void addEntityEvent(const lc::AddEntityEvent&);

            /*!
             * \brief Event to replace an Entity
             */
            void replaceEntityEvent(const lc::ReplaceEntityEvent&);

            /*!
             * \brief Event to remove an Entity
             */
            void removeEntityEvent(const lc::RemoveEntityEvent&);

            /*!
             * \brief Event to remove an layer
             */
            void removeLayerEvent(const lc::RemoveLayerEvent&);

            /*!
             * \brief Event to add a layer
             */
            void addLayerEvent(const lc::AddLayerEvent&);

            /*!
             * \brief Event to replace a layer
             */
            void replaceLayerEvent(const lc::ReplaceLayerEvent&);

        public:
            /*!
             * \brief Add a new Entity to the document
             * \param layerName Name of layer at which entity is to be added.
             * \param cadEntity Entity to be added.
             */
            virtual void insertEntity(const CADEntity_CSPtr cadEntity);
            /*!
             * \brief remove an Entity from the document
             * \param id Entity ID
             */
            virtual void removeEntity(CADEntity_CSPtr entity);

            virtual EntityContainer entitiesByLayer(const Layer_CSPtr layer);

            virtual EntityContainer entityContainer();
        protected:
            /*!
             * \brief execute's a operation
             * \param operation
             */
            void execute(operation::DocumentOperation_SPtr operation);

            /*!
             * \brief begins operation
             * \param operation
             */
            virtual void begin(operation::DocumentOperation_SPtr operation);
            /*!
             * \brief commits operation
             * \param operation
             */
            virtual void commit(operation::DocumentOperation_SPtr operation);

            virtual StorageManager_SPtr storageManager() const;

        private:
            std::mutex _documentMutex;
            const StorageManager_SPtr _storageManager;
    };
}

#endif


