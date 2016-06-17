
#include <string>
#include <unordered_map>
#include "documentimpl.h"

using namespace lc;

DocumentImpl::DocumentImpl(const StorageManager_SPtr storageManager) : Document() , _storageManager(storageManager) {

}

DocumentImpl::~DocumentImpl() {
    // LOG4CXX_DEBUG(logger, "DocumentImpl removed");
}

void DocumentImpl::execute(operation::DocumentOperation_SPtr operation) {
    std::lock_guard<std::mutex> lck(_documentMutex);
    begin(operation);
    this->operationProcess(operation);
    commit(operation);
}

void DocumentImpl::begin(operation::DocumentOperation_SPtr operation) {
    this->operationStart(operation);
    BeginProcessEvent event;
    beginProcessEvent()(event);
}

void DocumentImpl::commit(operation::DocumentOperation_SPtr operation) {
    _storageManager->optimise();
    CommitProcessEvent event(operation);
    commitProcessEvent()(event);

}

void DocumentImpl::insertEntity(const entity::CADEntity_CSPtr cadEntity) {
    if (_storageManager->entityByID(cadEntity->id()) != nullptr) {
        _storageManager->removeEntity(cadEntity);
        RemoveEntityEvent event(cadEntity);
        removeEntityEvent()(event);
    }

    _storageManager->insertEntity(cadEntity);
    AddEntityEvent event(cadEntity);
    addEntityEvent()(event);
}

void DocumentImpl::removeEntity(const entity::CADEntity_CSPtr entity) {
    _storageManager->removeEntity(entity);
    RemoveEntityEvent event(entity);
    removeEntityEvent()(event);
}



void DocumentImpl::addDocumentMetaType(const DocumentMetaType_CSPtr dmt) {
    _storageManager->addDocumentMetaType(dmt);

    auto layer = std::dynamic_pointer_cast<const Layer>(dmt);
    if (layer!=nullptr) {
        AddLayerEvent event(layer);
        addLayerEvent()(event);
    }
}
void DocumentImpl::removeDocumentMetaType(const DocumentMetaType_CSPtr dmt) {
    _storageManager->removeDocumentMetaType(dmt);
    auto layer = std::dynamic_pointer_cast<const Layer>(dmt);
    if (layer!=nullptr) {
        RemoveLayerEvent event(layer);
        removeLayerEvent()(event);
    }

}
void DocumentImpl::replaceDocumentMetaType(const DocumentMetaType_CSPtr oldDmt, const DocumentMetaType_CSPtr newDmt) {
    _storageManager->replaceDocumentMetaType(oldDmt, newDmt);
    auto oldLayer = std::dynamic_pointer_cast<const Layer>(oldDmt);
    if (oldLayer!=nullptr) {
        auto newLayer = std::dynamic_pointer_cast<const Layer>(newDmt);
        if (newLayer!=nullptr) {
            ReplaceLayerEvent event(oldLayer, newLayer);
            replaceLayerEvent()(event);
        } else {
            throw;
        }
    }
}

std::map<std::string, DocumentMetaType_CSPtr, lc::StringHelper::cmpCaseInsensetive> DocumentImpl::allMetaTypes() {
    return _storageManager->allMetaTypes();
}

EntityContainer<entity::CADEntity_CSPtr> DocumentImpl::entitiesByLayer(const Layer_CSPtr layer) {
    std::lock_guard<std::mutex> lck(_documentMutex);
    return _storageManager->entitiesByLayer(layer);
}

StorageManager_SPtr DocumentImpl::storageManager() const {
    return _storageManager;
}


EntityContainer<entity::CADEntity_CSPtr> DocumentImpl::entityContainer()  {
    //   std::lock_guard<std::mutex> lck(_documentMutex);
    return _storageManager->entityContainer();
}

std::map<std::string, Layer_CSPtr> DocumentImpl::allLayers() const {
    return _storageManager->allLayers();
}

Layer_CSPtr DocumentImpl::layerByName(const std::string& layerName) const {
    return _storageManager->layerByName(layerName);
}

DxfLinePattern_CSPtr DocumentImpl::linePatternByName(const std::string& linePatternName) const {
    return _storageManager->linePatternByName(linePatternName);
}
