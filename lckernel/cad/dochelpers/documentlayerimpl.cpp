#include "documentlayerimpl.h"

#include <QDebug>
#include <QVectorIterator>

using namespace lc;

DocumentLayerImpl::DocumentLayerImpl() {
}
DocumentLayerImpl::DocumentLayerImpl(shared_ptr<const Layer> layer) : _layer(layer) {
}

DocumentLayerImpl::~DocumentLayerImpl() {
    qDebug() << "DocumentLayerImpl removed";
}

shared_ptr<const Layer> DocumentLayerImpl::layer() const {
    return this->_layer;
}

void DocumentLayerImpl::addEntity(shared_ptr<const CADEntity> entity) {
    _cadentities.insert(entity->id(), entity);
}

void DocumentLayerImpl::removeEntity(ID_DATATYPE id) {
    if (_cadentities.contains(id)) {
        _cadentities.remove(id);
        return;
    }

    throw "Entity not found";
}

QHash<int, shared_ptr<const CADEntity> > DocumentLayerImpl::allEntities() const {
    return _cadentities;
}

shared_ptr<const CADEntity> DocumentLayerImpl::findByID(ID_DATATYPE id) const {
    if (_cadentities.contains(id)) {
        return _cadentities.value(id);
    }

    throw "Entity not found";
}
