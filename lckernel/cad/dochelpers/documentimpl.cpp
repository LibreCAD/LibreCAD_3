#include <string>
#include <unordered_map>
#include <memory>
#include <cad/meta/customentitystorage.h>
#include "documentimpl.h"
#include <cad/primitive/insert.h>
#include <cad/primitive/customentity.h>

using namespace lc;

DocumentImpl::DocumentImpl(StorageManager_SPtr storageManager) :
        Document() ,
        _storageManager(std::move(storageManager)) {
    _storageManager->addDocumentMetaType(std::make_shared<Layer>("0", MetaLineWidthByValue(1.0), Color(255, 255, 255)));
}

void DocumentImpl::execute(const operation::DocumentOperation_SPtr& operation) {
    {
        std::lock_guard<std::mutex> lck(_documentMutex);
        begin(operation);
        this->operationProcess(operation);
        commit(operation);

        _documentMutex.unlock();
    }

    auto tmp = _newWaitingCustomEntities;
    _newWaitingCustomEntities.clear();
    for (const auto& insert : tmp) {
        NewWaitingCustomEntityEvent customEntityEvent(insert);
        newWaitingCustomEntityEvent()(customEntityEvent);
    }
}

void DocumentImpl::begin(const operation::DocumentOperation_SPtr& operation) {
    this->operationStart(operation);
    BeginProcessEvent event;
    beginProcessEvent()(event);
}

void DocumentImpl::commit(const operation::DocumentOperation_SPtr& operation) {
    _storageManager->optimise();
    CommitProcessEvent event(operation);
    commitProcessEvent()(event);
}

void DocumentImpl::insertEntity(const entity::CADEntity_CSPtr& cadEntity) {
    if (_storageManager->entityByID(cadEntity->id()) != nullptr) {
        removeEntity(cadEntity);
    }

    _storageManager->insertEntity(cadEntity);
    AddEntityEvent event(cadEntity);
    addEntityEvent()(event);

    auto insert = std::dynamic_pointer_cast<const entity::Insert>(cadEntity);
    if (insert != nullptr && std::dynamic_pointer_cast<const entity::CustomEntity>(cadEntity) == nullptr) {
        auto ces = std::dynamic_pointer_cast<const CustomEntityStorage>(insert->displayBlock());

        if (ces != nullptr) {
            _waitingCustomEntities[ces->pluginName()].insert(insert);
            _newWaitingCustomEntities.insert(insert);
        }
    }
}

void DocumentImpl::removeEntity(const entity::CADEntity_CSPtr& entity) {
    auto insert = std::dynamic_pointer_cast<const entity::Insert>(entity);
    if (insert != nullptr && std::dynamic_pointer_cast<const entity::CustomEntity>(entity) == nullptr) {
        auto ces = std::dynamic_pointer_cast<const CustomEntityStorage>(insert->displayBlock());
        if (ces != nullptr) {
            _waitingCustomEntities[ces->pluginName()].erase(insert);
        }
    }

    if (_storageManager->entityByID(entity->id()) != nullptr) {
        _storageManager->removeEntity(entity);
        RemoveEntityEvent event(entity);
        removeEntityEvent()(event);
    }
}



void DocumentImpl::addDocumentMetaType(const DocumentMetaType_CSPtr& dmt) {
    _storageManager->addDocumentMetaType(dmt);

    auto layer = std::dynamic_pointer_cast<const Layer>(dmt);
    if (layer != nullptr) {
        AddLayerEvent event(layer);
        addLayerEvent()(event);
    }

    auto linePattern = std::dynamic_pointer_cast<const DxfLinePatternByValue>(dmt);
    if (linePattern != nullptr) {
        AddLinePatternEvent event(linePattern);
        addLinePatternEvent()(event);
    }
}
void DocumentImpl::removeDocumentMetaType(const DocumentMetaType_CSPtr& dmt) {
    _storageManager->removeDocumentMetaType(dmt);
    auto layer = std::dynamic_pointer_cast<const Layer>(dmt);
    if (layer != nullptr) {
        RemoveLayerEvent event(layer);
        removeLayerEvent()(event);
    }

    auto linePattern = std::dynamic_pointer_cast<const DxfLinePatternByValue>(dmt);
    if (linePattern != nullptr) {
        RemoveLinePatternEvent event(linePattern);
        removeLinePatternEvent()(event);
    }
}
void DocumentImpl::replaceDocumentMetaType(const DocumentMetaType_CSPtr& oldDmt, DocumentMetaType_CSPtr newDmt) {
    _storageManager->replaceDocumentMetaType(oldDmt, newDmt);
    auto oldLayer = std::dynamic_pointer_cast<const Layer>(oldDmt);
    if (oldLayer != nullptr) {
        auto newLayer = std::dynamic_pointer_cast<const Layer>(newDmt);
        if (newLayer != nullptr) {
            ReplaceLayerEvent event(oldLayer, newLayer);
            replaceLayerEvent()(event);
        } else {
            throw;
        }
    }

    auto oldLinePattern = std::dynamic_pointer_cast<const DxfLinePatternByValue>(oldDmt);
    if (oldLinePattern != nullptr) {
        auto newLinePattern = std::dynamic_pointer_cast<const DxfLinePatternByValue>(newDmt);
        if (newLinePattern != nullptr) {
            ReplaceLinePatternEvent event(oldLinePattern, newLinePattern);
            replaceLinePatternEvent()(event);
        }
    }
}

std::map<std::string, DocumentMetaType_CSPtr, lc::StringHelper::cmpCaseInsensetive> DocumentImpl::allMetaTypes() {
    return _storageManager->allMetaTypes();
}

EntityContainer<entity::CADEntity_CSPtr> DocumentImpl::entitiesByLayer(const Layer_CSPtr& layer) {
    std::lock_guard<std::mutex> lck(_documentMutex);
    return _storageManager->entitiesByLayer(layer);
}

EntityContainer<entity::CADEntity_CSPtr>& DocumentImpl::entityContainer() {
    //   std::lock_guard<std::mutex> lck(_documentMutex);
    return _storageManager->entityContainer();
}

std::map<std::string, Layer_CSPtr> DocumentImpl::allLayers() const {
    return _storageManager->allLayers();
}

Layer_CSPtr DocumentImpl::layerByName(const std::string& layerName) const {
    return _storageManager->layerByName(layerName);
}

DxfLinePatternByValue_CSPtr DocumentImpl::linePatternByName(const std::string& linePatternName) const {
    return _storageManager->linePatternByName(linePatternName);
}

/**
 * Return all line patterns
 * @todo probably change this to metaTypes<T>()
 */
std::vector<DxfLinePatternByValue_CSPtr> DocumentImpl::linePatterns() const {
    return _storageManager->metaTypes<const DxfLinePatternByValue>();
}

EntityContainer<entity::CADEntity_CSPtr> DocumentImpl::entitiesByBlock(const Block_CSPtr& block) {
    return _storageManager->entitiesByBlock(block);
}

std::vector<Block_CSPtr> DocumentImpl::blocks() const {
    return _storageManager->metaTypes<const Block>();
}

std::unordered_set<entity::Insert_CSPtr> DocumentImpl::waitingCustomEntities(const std::string& pluginName) {
    return _waitingCustomEntities[pluginName];
}
