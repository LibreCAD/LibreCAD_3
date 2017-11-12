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
            virtual void insertEntity(const entity::CADEntity_CSPtr cadEntity) override;
            virtual void removeEntity(entity::CADEntity_CSPtr entity) override;

            virtual void addDocumentMetaType(const DocumentMetaType_CSPtr dmt) override;
            virtual void removeDocumentMetaType(const DocumentMetaType_CSPtr dmt) override;
            virtual void replaceDocumentMetaType(const DocumentMetaType_CSPtr oldDmt, const DocumentMetaType_CSPtr newDmt) override;
            virtual std::map<std::string, DocumentMetaType_CSPtr, lc::StringHelper::cmpCaseInsensetive> allMetaTypes() override;
            /**
             * @brief entitiesByLayer
             * @param layer
             * @return
             */
            virtual EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const Layer_CSPtr layer) override;

            EntityContainer<entity::CADEntity_CSPtr> entitiesByBlock(const Block_CSPtr block) override;

            virtual EntityContainer<entity::CADEntity_CSPtr> entityContainer() override;

            virtual std::map<std::string, Layer_CSPtr> allLayers() const override;

            virtual Layer_CSPtr layerByName(const std::string& layerName) const override;
            virtual DxfLinePatternByValue_CSPtr linePatternByName(const std::string& linePatternName) const override;

            /**
             * @brief return all line patterns
             */
            virtual std::vector<DxfLinePatternByValue_CSPtr> linePatterns() const override;
        protected:
            /*!
             * \brief execute's a operation
             * \param operation
             */
            void execute(operation::DocumentOperation_SPtr operation) override;

            /*!
             * \brief begins operation
             * \param operation
             */
            virtual void begin(operation::DocumentOperation_SPtr operation) override;
            /*!
             * \brief commits operation
             * \param operation
             */
            virtual void commit(operation::DocumentOperation_SPtr operation) override;

            virtual StorageManager_SPtr storageManager() const;

        public:
            std::unordered_set<entity::Insert_CSPtr> waitingCustomEntities(const std::string& pluginName) override;

            std::vector<Block_CSPtr> blocks() const override;

        private:
            std::mutex _documentMutex;
            // AI am considering remove the shared_ptr from this one so we can never get a shared object from it
            StorageManager_SPtr _storageManager;
            std::thread _testThread;

            std::map<std::string, std::unordered_set<entity::Insert_CSPtr>> _waitingCustomEntities;
            std::unordered_set<entity::Insert_CSPtr> _newWaitingCustomEntities;

    };
}
