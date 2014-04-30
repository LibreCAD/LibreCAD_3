
#include <QHashIterator>
#include <QVector>
#include "qdebug.h"

#include "documentimpl.h"
#include "cad/geometry/geocoordinate.h"



using namespace lc;

DocumentImpl::DocumentImpl(const std::shared_ptr<StorageManager> storageManager) : Document() , _storageManager(storageManager) {
}

DocumentImpl::~DocumentImpl() {
    qDebug() << "DocumentImpl removed";
}

void DocumentImpl::execute(std::shared_ptr<operation::DocumentOperation> operation) {
    std::lock_guard<std::mutex> lck(_documentMutex);
    begin(operation);
    this->operationProcess(operation);
    commit(operation);
}

void DocumentImpl::begin(std::shared_ptr<operation::DocumentOperation> operation) {
    this->operationStart(operation);
    BeginProcessEvent event;
    emit beginProcessEvent(event);
}

void DocumentImpl::commit(std::shared_ptr<operation::DocumentOperation> operation) {
    CommitProcessEvent event(operation);
    emit commitProcessEvent(event);
    this->operationFinnish(operation);
}

void DocumentImpl::insertEntity(const std::shared_ptr<const CADEntity> cadEntity) {
    if (_storageManager->entityByID(cadEntity->id()).get() != NULL) {
        _storageManager->removeEntity(cadEntity);
        RemoveEntityEvent event(cadEntity);
        emit removeEntityEvent(cadEntity);
    }

    _storageManager->insertEntity(cadEntity);
    AddEntityEvent event(cadEntity);
    emit addEntityEvent(event);
}

void DocumentImpl::removeEntity(const std::shared_ptr<const CADEntity> entity) {
    _storageManager->removeEntity(entity);
    RemoveEntityEvent event(entity);
    emit removeEntityEvent(event);
}

EntityContainer DocumentImpl::entitiesByLayer(const std::shared_ptr<const Layer> layer) const {
    return _storageManager->entitiesByLayer(layer);;
}

void DocumentImpl::test(const std::shared_ptr<const Layer> layer) const {
    entitiesByLayer(layer);
}

std::shared_ptr<StorageManager> DocumentImpl::storageManager() const {
    return _storageManager;
}


