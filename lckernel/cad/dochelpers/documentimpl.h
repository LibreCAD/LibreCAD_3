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
            virtual void insertEntity(const CADEntity_CSPtr cadEntity);
            virtual void removeEntity(CADEntity_CSPtr entity);

            virtual void addLayer(const Layer_CSPtr layer);
            virtual void removeLayer(const Layer_CSPtr layer);
            virtual void replaceLayer(const Layer_CSPtr oldLayer, const Layer_CSPtr newLayer);

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
