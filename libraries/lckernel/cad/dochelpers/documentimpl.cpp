
#include <QHashIterator>
#include <QVector>
#include <qDebug>

#include "documentimpl.h"
#include "cad/geometry/geocoordinate.h"



using namespace lc;

DocumentImpl::DocumentImpl(LayerManager* layerManager, EntityManager* entityManager) : AbstractDocument() {
    _layerManager = layerManager;
    layerManager->setDocument(this);
    entityManager->setDocument(this);
    releaseLock();
}

DocumentImpl::~DocumentImpl() {
    delete _layerManager;
}

LayerManager* DocumentImpl::layerManager() const {
    return _layerManager;
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

    qDebug() << "findEntityByID: " << id;

    while (li.hasNext()) {
        li.next();
        DocumentLayerPtr documentLayer = li.value();

        QListIterator<CADEntityPtr> cad(documentLayer->allEntities());
        while (cad.hasNext()) {
           CADEntityPtr cip=cad.next();
           if (cip->id() == id) {
               return cip;
           }
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

        QListIterator<CADEntityPtr> cad(documentLayer->allEntities());
        while (cad.hasNext()) {
           CADEntityPtr cip=cad.next();
           if (cip->id() == id) {
               return documentLayer->layer()->name();
           }
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

