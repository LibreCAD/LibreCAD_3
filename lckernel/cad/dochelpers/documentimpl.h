#pragma once

#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include "cad/const.h"

#include "cad/document/document.h"
#include "cad/document/storagemanager.h"
#include "cad/document/selectionmanager.h"
#include "cad/operations/documentoperation.h"

namespace lc {

    class DocumentImpl : public Document {
        public:
            DocumentImpl(const StorageManager_SPtr storageManager);
            virtual ~DocumentImpl();

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

            /**
             * @brief entitiesByLayer
             * @param layer
             * @return
             */
            virtual EntityContainer<CADEntity_CSPtr> entitiesByLayer(const Layer_CSPtr layer);

            virtual EntityContainer<CADEntity_CSPtr> entityContainer();
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
