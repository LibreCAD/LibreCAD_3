
#include <QHashIterator>
#include <QVector>
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
    // FIXME: during a undo cycle we can get duplicate entries in _allEntites, we should clean this list up right before commit and create a list of single entries
    _allEntites.append(cadEntity);
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


CADEntity* AbstractDocumentImpl::findByID(ID_DATATYPE id) const {
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







void AbstractDocumentImpl::lock() {
    _locked = true;
}
void AbstractDocumentImpl::releaseLock() {
    _locked = false;
}

