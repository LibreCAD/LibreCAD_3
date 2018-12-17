#pragma once

#include <mutex>
#include <vector>
#include <string>
#include <unordered_set>
#include <map>
#include "cad/const.h"

#include "document.h"
#include "storagemanager.h"
#include "cad/operations/documentoperation.h"

namespace lc {
    namespace storage {
        class DocumentImpl : public Document {
            public:
                DocumentImpl(StorageManager_SPtr storageManager);

                virtual ~DocumentImpl() = default;

            public:
                void insertEntity(const entity::CADEntity_CSPtr& cadEntity) override;

                void removeEntity(const entity::CADEntity_CSPtr& entity) override;

                void addDocumentMetaType(const meta::DocumentMetaType_CSPtr& dmt) override;

                void removeDocumentMetaType(const meta::DocumentMetaType_CSPtr& dmt) override;

                void
                replaceDocumentMetaType(const meta::DocumentMetaType_CSPtr& oldDmt, meta::DocumentMetaType_CSPtr newDmt) override;

                std::map<std::string, meta::DocumentMetaType_CSPtr, lc::tools::StringHelper::cmpCaseInsensetive>
                allMetaTypes() override;

                /**
                 * @brief entitiesByLayer
                 * @param layer
                 * @return
                 */
                EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const meta::Layer_CSPtr& layer) override;

                EntityContainer<entity::CADEntity_CSPtr> entitiesByBlock(const meta::Block_CSPtr& block) override;

                EntityContainer<entity::CADEntity_CSPtr>& entityContainer() override;

                std::map<std::string, meta::Layer_CSPtr> allLayers() const override;

                meta::Layer_CSPtr layerByName(const std::string& layerName) const override;

                EntityContainer<entity::CADEntity_CSPtr> entitiesByViewport(const meta::Viewport_CSPtr& viewport) override;

                std::map<std::string, meta::Viewport_CSPtr> allViewports() const override;

                meta::Viewport_CSPtr viewportByName(const std::string& viewportName) const override;

                meta::DxfLinePatternByValue_CSPtr linePatternByName(const std::string& linePatternName) const override;

                /**
                 * @brief return all line patterns
                 */
                std::vector<meta::DxfLinePatternByValue_CSPtr> linePatterns() const override;

            protected:
                /*!
                 * \brief execute's a operation
                 * \param operation
                 */
                void execute(const operation::DocumentOperation_SPtr& operation) override;

                /*!
                 * \brief begins operation
                 * \param operation
                 */
                void begin(const operation::DocumentOperation_SPtr& operation) override;

                /*!
                 * \brief commits operation
                 * \param operation
                 */
                void commit(const operation::DocumentOperation_SPtr& operation) override;

            public:
                std::unordered_set<entity::Insert_CSPtr> waitingCustomEntities(const std::string& pluginName) override;

                std::vector<lc::meta::Block_CSPtr> blocks() const override;

            private:
                std::mutex _documentMutex;
                // AI am considering remove the shared_ptr from this one so we can never get a shared object from it
                StorageManager_SPtr _storageManager;

                std::map<std::string, std::unordered_set<entity::Insert_CSPtr>> _waitingCustomEntities;
                std::unordered_set<entity::Insert_CSPtr> _newWaitingCustomEntities;
        };
    }
}
