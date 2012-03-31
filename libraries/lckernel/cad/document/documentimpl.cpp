
#include <QHashIterator>
#include <QVector>
#include "documentimpl.h"
#include "cad/geometry/coordinate.h"



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

void DocumentImpl::operateOn(Operation* operation) {
    begin(operation);
    this->operationProcess(operation);
    commit(operation);
}

void DocumentImpl::begin(Operation* operation) {
    lock();
    this->operationStart(operation);
    BeginProcessEvent event;
    emit beginProcessEvent(& event);
}

void DocumentImpl::commit(Operation* operation) {
    CommitProcessEvent event(operation);
    emit commitProcessEvent(& event);
    this->operationFinnish(operation);
    releaseLock();
}

void DocumentImpl::addEntity(const QString& layerName, CADEntity* cadEntity) {
    // FIXME: during a undo cycle we can get duplicate entries in _allEntites, we should clean this list up right before commit and create a list of single entries
    _allEntites.append(cadEntity);
    AddEntityEvent event(layerName, cadEntity);
    emit addEntityEvent(& event);
}

void DocumentImpl::replaceEntity(CADEntity* oldEntity, CADEntity* newEntity) {
    ReplaceEntityEvent event(oldEntity, newEntity);
    emit replaceEntityEvent(& event);
}
void DocumentImpl::removeEntity(ID_DATATYPE id) {
    RemoveEntityEvent event(id);
    emit removeEntityEvent(& event);
}


CADEntity* DocumentImpl::findEntityByID(ID_DATATYPE id) const {
    QHash <QString, DocumentLayer*>* allLayers = layerManager()->allLayers();
    QHashIterator<QString, DocumentLayer*> li(*allLayers);

    while (li.hasNext()) {
        DocumentLayer* documentLayer = li.value();

        QVector<CADEntity*>* _cadentities = documentLayer->allEntities();
        long unsigned int size = _cadentities->size();

        for (long unsigned int idx = 0; idx < size; ++idx) {
            if (_cadentities->at(idx)->id() == id) {
                return _cadentities->at(idx);
            }
        }
    }

    throw "Entity not found";
    return NULL;
}

QString DocumentImpl::findEntityLayerByID(ID_DATATYPE id) const {
    QHash <QString, DocumentLayer*>* allLayers = layerManager()->allLayers();
    QHashIterator<QString, DocumentLayer*> li(*allLayers);

    while (li.hasNext()) {
        DocumentLayer* documentLayer = li.value();

        QVector<CADEntity*>* _cadentities = documentLayer->allEntities();
        long unsigned int size = _cadentities->size();

        for (long unsigned int idx = 0; idx < size; ++idx) {
            if (_cadentities->at(idx)->id() == id) {
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

