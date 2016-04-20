#pragma once

#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include "cad/const.h"

#include "cad/document/document.h"
#include "cad/document/storagemanager.h"
#include "cad/operations/documentoperation.h"

namespace lc {

    class DocumentImpl : public Document {
        public:
            DocumentImpl(const StorageManager_SPtr storageManager);
            virtual ~DocumentImpl();

        public:
            virtual void insertEntity(const entity::CADEntity_CSPtr cadEntity);
            virtual void removeEntity(entity::CADEntity_CSPtr entity);

            virtual void addDocumentMetaType(const DocumentMetaType_CSPtr dmt);
            virtual void removeDocumentMetaType(const DocumentMetaType_CSPtr dmt);
            virtual void replaceDocumentMetaType(const DocumentMetaType_CSPtr oldDmt, const DocumentMetaType_CSPtr newDmt);

            /**
             * @brief entitiesByLayer
             * @param layer
             * @return
             */
            virtual EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const Layer_CSPtr layer);

            virtual EntityContainer<entity::CADEntity_CSPtr> entityContainer();

            virtual Layer_CSPtr layerByName(const std::string& layerName) const;
            virtual DxfLinePattern_CSPtr linePatternByName(const std::string& linePatternName) const;
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
            // AI am considering remove the shared_ptr from this one so we can never get a shared object from it
            StorageManager_SPtr _storageManager;

    };
}
