#include "documentimpl.h"
#include "cad/geometry/coordinate.h"



using namespace lc;

DocumentImpl::DocumentImpl(LayerManager* layerManager) : Document() {
    _layerManager = layerManager;
    releaseLock();
}

DocumentImpl::~DocumentImpl() {
    delete _layerManager;
}



LayerManager* DocumentImpl::layerManager() const {
    return _layerManager;
}





void DocumentImpl::operateOn(Operation* operation)
{
    begin(operation);
    this->operationProcess(operation);
    commit(operation);
}

void DocumentImpl::begin(Operation* operation)
{
    lock();
    this->operationStart(operation);
    BeginProcessEvent event;
    emit beginProcessEvent(& event);
}

void DocumentImpl::commit(Operation* operation)
{
    CommitProcessEvent event;
    emit commitProcessEvent(& event);
    this->operationFinnish(operation);
    releaseLock();
}

void DocumentImpl::addEntity(CADEntity* cadEntity)
{
    AddEntityEvent event(cadEntity);
    emit addEntityEvent(& event);
}

void DocumentImpl::replaceEntity(CADEntity* oldEntity, CADEntity* newEntity)
{
    ReplaceEntityEvent event(oldEntity, newEntity);
    emit replaceEntityEvent(& event);
}
void DocumentImpl::removeEntity(ID_DATATYPE id)
{
    RemoveEntityEvent event(id);
    emit removeEntityEvent(& event);
}






void DocumentImpl::lock() {
    _locked = true;
}
void DocumentImpl::releaseLock() {
    _locked = false;
}

