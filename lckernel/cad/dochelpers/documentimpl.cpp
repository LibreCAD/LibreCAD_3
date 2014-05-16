
#include <QHashIterator>
#include <QVector>
#include "qdebug.h"

#include "documentimpl.h"
#include "cad/geometry/geocoordinate.h"



using namespace lc;

DocumentImpl::DocumentImpl(const StorageManager_SPtr storageManager) : Document() , _storageManager(storageManager) {
}

DocumentImpl::~DocumentImpl() {
    qDebug() << "DocumentImpl removed";
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
    emit beginProcessEvent(event);
}

void DocumentImpl::commit(operation::DocumentOperation_SPtr operation) {
    CommitProcessEvent event(operation);
    emit commitProcessEvent(event);
    this->operationFinnish(operation);
}

void DocumentImpl::insertEntity(const CADEntity_CSPtr cadEntity) {
    if (_storageManager->entityByID(cadEntity->id()).get() != nullptr) {
        _storageManager->removeEntity(cadEntity);
        RemoveEntityEvent event(cadEntity);
        emit removeEntityEvent(cadEntity);
    }

    _storageManager->insertEntity(cadEntity);
    AddEntityEvent event(cadEntity);
    emit addEntityEvent(event);
}

void DocumentImpl::removeEntity(const CADEntity_CSPtr entity) {
    _storageManager->removeEntity(entity);
    RemoveEntityEvent event(entity);
    emit removeEntityEvent(event);
}

EntityContainer DocumentImpl::entitiesByLayer(const Layer_CSPtr layer) {
    std::lock_guard<std::mutex> lck(_documentMutex);
    return _storageManager->entitiesByLayer(layer);;
}

StorageManager_SPtr DocumentImpl::storageManager() const {
    return _storageManager;
}


EntityContainer DocumentImpl::entityContainer()  {
    std::lock_guard<std::mutex> lck(_documentMutex);
    return _storageManager->entityContainer();
}
