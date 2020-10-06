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

    EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const meta::Layer_CSPtr& layer) override;

    EntityContainer<entity::CADEntity_CSPtr> entitiesByBlock(const meta::Block_CSPtr& block) override;

    EntityContainer<entity::CADEntity_CSPtr>& entityContainer() override;

    std::map<std::string, meta::Layer_CSPtr> allLayers() const override;

    meta::Layer_CSPtr layerByName(const std::string& layerName) const override;

    meta::Block_CSPtr blockByName(const std::string& blockName) const override;

    meta::DxfLinePatternByValue_CSPtr linePatternByName(const std::string& linePatternName) const override;

    std::vector<meta::DxfLinePatternByValue_CSPtr> linePatterns() const override;

    entity::CADEntity_CSPtr entityByID(ID_DATATYPE id) const override;

protected:
    void execute(const operation::DocumentOperation_SPtr& operation) override;

    void begin(const operation::DocumentOperation_SPtr& operation) override;

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
