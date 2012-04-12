
#include <QHashIterator>
#include <QVector>
#include "qdebug.h"

#include "documentimpl.h"
#include "cad/geometry/geocoordinate.h"



using namespace lc;

DocumentImpl::DocumentImpl() : AbstractDocument() {
    _layerManager = NULL;
    releaseLock();
}

DocumentImpl::~DocumentImpl() {
}

LayerManager* DocumentImpl::layerManager() const {
    return _layerManager;
}

void DocumentImpl::setLayerManager(LayerManager* layerManager) {
    _layerManager = layerManager;
}

void DocumentImpl::operateOn(OperationPtr operation) {
    begin(operation);
    this->operationProcess(operation);
    commit(operation);
}

void DocumentImpl::begin(OperationPtr operation) {
    lock();
    this->operationStart(operation);
    BeginProcessEvent event;
    emit beginProcessEvent(event);
}

void DocumentImpl::commit(OperationPtr operation) {
    CommitProcessEvent event(operation);
    emit commitProcessEvent(event);
    this->operationFinnish(operation);
    releaseLock();
}

void DocumentImpl::addEntity(const QString& layerName, CADEntityPtr cadEntity) {
    AddEntityEvent event(layerName, cadEntity);
    emit addEntityEvent(event);
}

void DocumentImpl::replaceEntity(CADEntityPtr oldEntity, CADEntityPtr newEntity) {
    ReplaceEntityEvent event(oldEntity, newEntity);
    emit replaceEntityEvent(event);
}
void DocumentImpl::removeEntity(ID_DATATYPE id) {
    RemoveEntityEvent event(id);
    emit removeEntityEvent(event);
}
void DocumentImpl::absoleteEntity(CADEntityPtr entity) {
    AbsoluteEntityEvent event(entity);
    emit absoleteEntityEvent(event);
}


CADEntityPtr DocumentImpl::findEntityByID(ID_DATATYPE id) const {
    QHash <QString, DocumentLayerPtr> allLayers = layerManager()->allLayers();
    QHashIterator<QString, DocumentLayerPtr> li(allLayers);

    while (li.hasNext()) {
        li.next();
        DocumentLayerPtr documentLayer = li.value();

        try {
            CADEntityPtr cip = documentLayer->findByID(id);
        } catch (QString error) {
            //
        }
    }

    throw "Entity not found";
}

QString DocumentImpl::findEntityLayerByID(ID_DATATYPE id) const {
    QHash <QString, DocumentLayerPtr> allLayers = layerManager()->allLayers();
    QHashIterator<QString, DocumentLayerPtr> li(allLayers);

    while (li.hasNext()) {
        li.next();
        DocumentLayerPtr documentLayer = li.value();

        try {
            CADEntityPtr cip = documentLayer->findByID(id);
            return documentLayer->layer()->name();
        } catch (QString error) {
            //
        }
    }

    throw "Entity not found";
    return "";
}






void DocumentImpl::lock() {
    _locked = true;
}
void DocumentImpl::releaseLock() {
    _locked = false;
}

