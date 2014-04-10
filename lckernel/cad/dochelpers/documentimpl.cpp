
#include <QHashIterator>
#include <QVector>
#include "qdebug.h"

#include "documentimpl.h"
#include "cad/geometry/geocoordinate.h"



using namespace lc;

DocumentImpl::DocumentImpl() : AbstractDocument() {
}

DocumentImpl::~DocumentImpl() {
    qDebug() << "DocumentImpl removed";
}

shared_ptr<lc::LayerManager> DocumentImpl::layerManager() const {
    return _layerManager;
}

void DocumentImpl::setLayerManager(shared_ptr<lc::LayerManager> layerManager) {
    _layerManager = layerManager;
}

void DocumentImpl::operateOn(shared_ptr<lc::Operation> operation) {
    QMutexLocker locker(&_documentMutex);
    begin(operation);
    this->operationProcess(operation);
    commit(operation);
}

void DocumentImpl::begin(shared_ptr<lc::Operation> operation) {
    this->operationStart(operation);
    BeginProcessEvent event;
    emit beginProcessEvent(event);
}

void DocumentImpl::commit(shared_ptr<lc::Operation> operation) {
    CommitProcessEvent event(operation);
    emit commitProcessEvent(event);
    this->operationFinnish(operation);
}

void DocumentImpl::addEntity(const QString& layerName, shared_ptr<const lc::CADEntity> cadEntity) {
    AddEntityEvent event(layerName, cadEntity);
    emit addEntityEvent(event);
}

void DocumentImpl::replaceEntity(shared_ptr<const lc::CADEntity> oldEntity, shared_ptr<const lc::CADEntity> newEntity) {
    ReplaceEntityEvent event(oldEntity, newEntity);
    emit replaceEntityEvent(event);
}
void DocumentImpl::removeEntity(ID_DATATYPE id) {
    RemoveEntityEvent event(id);
    emit removeEntityEvent(event);
}
void DocumentImpl::absoleteEntity(shared_ptr<const lc::CADEntity> entity) {
    AbsoluteEntityEvent event(entity);
    emit absoleteEntityEvent(event);
}


shared_ptr<const lc::CADEntity> DocumentImpl::findEntityByID(ID_DATATYPE id) const {
    QHash <QString, shared_ptr<lc::DocumentLayer> > allLayers = layerManager()->allLayers();
    QHashIterator<QString, shared_ptr<lc::DocumentLayer> > li(allLayers);

    while (li.hasNext()) {
        li.next();
        auto documentLayer = li.value();

        try {
            shared_ptr<const lc::CADEntity> cip = documentLayer->findByID(id);
        } catch (QString error) {
            //
        }
    }

    throw "Entity not found";
}

QString DocumentImpl::findEntityLayerByID(ID_DATATYPE id) const {
    QHash <QString, shared_ptr<lc::DocumentLayer> > allLayers = layerManager()->allLayers();
    QHashIterator<QString, shared_ptr<lc::DocumentLayer> > li(allLayers);

    while (li.hasNext()) {
        li.next();
        shared_ptr<lc::DocumentLayer> documentLayer = li.value();

        try {
            shared_ptr<const lc::CADEntity> cip = documentLayer->findByID(id);
            return documentLayer->layer()->name();
        } catch (QString error) {
            //
        }
    }

    throw "Entity not found";
    return "";
}



