
#include <string>
#include <unordered_map>
#include "documentimpl.h"
#include "cad/geometry/geocoordinate.h"

#include "nano-signal-slot/nano_signal_slot.hpp"


using namespace lc;

DocumentImpl::DocumentImpl(const StorageManager_SPtr storageManager) : Document() , _storageManager(storageManager) {

}

DocumentImpl::~DocumentImpl() {
    LOG4CXX_DEBUG(logger, "DocumentImpl removed");
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

void DocumentImpl::insertEntity(const CADEntity_CSPtr cadEntity) {
    if (_storageManager->entityByID(cadEntity->id()).get() != nullptr) {
        _storageManager->removeEntity(cadEntity);
        RemoveEntityEvent event(cadEntity);
        removeEntityEvent()(event);
    }

    _storageManager->insertEntity(cadEntity);
    AddEntityEvent event(cadEntity);
    addEntityEvent()(event);
}

void DocumentImpl::removeEntity(const CADEntity_CSPtr entity) {
    _storageManager->removeEntity(entity);
    RemoveEntityEvent event(entity);
    removeEntityEvent()(event);
}

EntityContainer<CADEntity_CSPtr> DocumentImpl::entitiesByLayer(const Layer_CSPtr layer) {
    std::lock_guard<std::mutex> lck(_documentMutex);
    return _storageManager->entitiesByLayer(layer);;
}

StorageManager_SPtr DocumentImpl::storageManager() const {
    return _storageManager;
}


EntityContainer<CADEntity_CSPtr> DocumentImpl::entityContainer()  {
    std::lock_guard<std::mutex> lck(_documentMutex);
    return _storageManager->entityContainer();
}
