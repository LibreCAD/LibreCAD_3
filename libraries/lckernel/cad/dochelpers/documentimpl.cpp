
#include <QHashIterator>
#include <QVector>
#include "qdebug.h"

#include "documentimpl.h"
#include "cad/geometry/geocoordinate.h"



using namespace lc;

DocumentImpl::DocumentImpl() : AbstractDocument() {
    releaseLock();
}

DocumentImpl::~DocumentImpl() {
}

std::tr1::shared_ptr<lc::LayerManager> DocumentImpl::layerManager() const {
    return _layerManager;
}

void DocumentImpl::setLayerManager(std::tr1::shared_ptr<lc::LayerManager> layerManager) {
    _layerManager = layerManager;
}

void DocumentImpl::operateOn(std::tr1::shared_ptr<lc::Operation> operation) {
    begin(operation);
    this->operationProcess(operation);
    commit(operation);
}

void DocumentImpl::begin(std::tr1::shared_ptr<lc::Operation> operation) {
    lock();
    this->operationStart(operation);
    BeginProcessEvent event;
    emit beginProcessEvent(event);
}

void DocumentImpl::commit(std::tr1::shared_ptr<lc::Operation> operation) {
    CommitProcessEvent event(operation);
    emit commitProcessEvent(event);
    this->operationFinnish(operation);
    releaseLock();
}

void DocumentImpl::addEntity(const QString& layerName, std::tr1::shared_ptr<const lc::CADEntity> cadEntity) {
    AddEntityEvent event(layerName, cadEntity);
    emit addEntityEvent(event);
}

void DocumentImpl::replaceEntity(std::tr1::shared_ptr<const lc::CADEntity> oldEntity, std::tr1::shared_ptr<const lc::CADEntity> newEntity) {
    ReplaceEntityEvent event(oldEntity, newEntity);
    emit replaceEntityEvent(event);
}
void DocumentImpl::removeEntity(ID_DATATYPE id) {
    RemoveEntityEvent event(id);
    emit removeEntityEvent(event);
}
void DocumentImpl::absoleteEntity(std::tr1::shared_ptr<const lc::CADEntity> entity) {
    AbsoluteEntityEvent event(entity);
    emit absoleteEntityEvent(event);
}


std::tr1::shared_ptr<const lc::CADEntity> DocumentImpl::findEntityByID(ID_DATATYPE id) const {
    QHash <QString, std::tr1::shared_ptr<lc::DocumentLayer> > allLayers = layerManager()->allLayers();
    QHashIterator<QString, std::tr1::shared_ptr<lc::DocumentLayer> > li(allLayers);

    while (li.hasNext()) {
        li.next();
        std::tr1::shared_ptr<lc::DocumentLayer> documentLayer = li.value();

        try {
            std::tr1::shared_ptr<const lc::CADEntity> cip = documentLayer->findByID(id);
        } catch (QString error) {
            //
        }
    }

    throw "Entity not found";
}

QString DocumentImpl::findEntityLayerByID(ID_DATATYPE id) const {
    QHash <QString, std::tr1::shared_ptr<lc::DocumentLayer> > allLayers = layerManager()->allLayers();
    QHashIterator<QString, std::tr1::shared_ptr<lc::DocumentLayer> > li(allLayers);

    while (li.hasNext()) {
        li.next();
        std::tr1::shared_ptr<lc::DocumentLayer> documentLayer = li.value();

        try {
            std::tr1::shared_ptr<const lc::CADEntity> cip = documentLayer->findByID(id);
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

