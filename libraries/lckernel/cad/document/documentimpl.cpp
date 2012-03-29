#include "documentimpl.h"
#include "cad/geometry/coordinate.h"



using namespace lc;

AbstractDocumentImpl::AbstractDocumentImpl(LayerManager* layerManager, EntityManager* entityManager) : AbstractDocument() {
    _layerManager = layerManager;
    layerManager->document(this);
    entityManager->document(this);
    releaseLock();
}

AbstractDocumentImpl::~AbstractDocumentImpl() {
    delete _layerManager;
}

LayerManager* AbstractDocumentImpl::layerManager() const {
    return _layerManager;
}

void AbstractDocumentImpl::operateOn(Operation* operation) {
    begin(operation);
    this->operationProcess(operation);
    commit(operation);
}

void AbstractDocumentImpl::begin(Operation* operation) {
    lock();
    this->operationStart(operation);
    BeginProcessEvent event;
    emit beginProcessEvent(& event);
}

void AbstractDocumentImpl::commit(Operation* operation) {
    CommitProcessEvent event;
    emit commitProcessEvent(& event);
    this->operationFinnish(operation);
    releaseLock();
}

void AbstractDocumentImpl::addEntity(const QString& layerName, CADEntity* cadEntity) {
    AddEntityEvent event(layerName, cadEntity);
    emit addEntityEvent(& event);
}

void AbstractDocumentImpl::replaceEntity(CADEntity* oldEntity, CADEntity* newEntity) {
    ReplaceEntityEvent event(oldEntity, newEntity);
    emit replaceEntityEvent(& event);
}
void AbstractDocumentImpl::removeEntity(ID_DATATYPE id) {
    RemoveEntityEvent event(id);
    emit removeEntityEvent(& event);
}






void AbstractDocumentImpl::lock() {
    _locked = true;
}
void AbstractDocumentImpl::releaseLock() {
    _locked = false;
}

