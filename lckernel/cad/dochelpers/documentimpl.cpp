
#include <QHashIterator>
#include <QVector>
#include "qdebug.h"

#include "documentimpl.h"
#include "cad/geometry/geocoordinate.h"



using namespace lc;

DocumentImpl::DocumentImpl() : Document() {
}

DocumentImpl::~DocumentImpl() {
    qDebug() << "DocumentImpl removed";
}

void DocumentImpl::execute(shared_ptr<operation::DocumentOperation> operation) {
    std::lock_guard<std::mutex> lck(_documentMutex);
    begin(operation);
    this->operationProcess(operation);
    commit(operation);
}

void DocumentImpl::begin(shared_ptr<operation::DocumentOperation> operation) {
    this->operationStart(operation);
    BeginProcessEvent event;
    emit beginProcessEvent(event);
}

void DocumentImpl::commit(shared_ptr<operation::DocumentOperation> operation) {
    CommitProcessEvent event(operation);
    emit commitProcessEvent(event);
    this->operationFinnish(operation);
}

void DocumentImpl::addEntity(const shared_ptr<const CADEntity> cadEntity) {
    AddEntityEvent event(cadEntity);
    emit addEntityEvent(event);
}

void DocumentImpl::replaceEntity(const shared_ptr<const CADEntity> entity) {
    ReplaceEntityEvent event(entity);
    emit replaceEntityEvent(entity);
}
void DocumentImpl::removeEntity(const shared_ptr<const CADEntity> entity) {
    RemoveEntityEvent event(entity);
    emit removeEntityEvent(event);
}




