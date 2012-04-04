#include "documentlayerimpl.h"

#include <QDebug>
#include <QVectorIterator>

using namespace lc;

DocumentLayerImpl::DocumentLayerImpl() {
    _layer = NULL;
}
DocumentLayerImpl::DocumentLayerImpl(Layer* layer) {
    _layer = layer;
}

DocumentLayerImpl::~DocumentLayerImpl() {
}

Layer* DocumentLayerImpl::layer() const {
    return this->_layer;
}

void DocumentLayerImpl::addEntity(CADEntityPtr entity) {
    _cadentities.insert(entity->id(), entity);
}

void DocumentLayerImpl::removeEntity(ID_DATATYPE id) {
    if (_cadentities.contains(id)) {
        _cadentities.remove(id);
        return;
    }

    throw "Entity not found";
}

QHash<int, CADEntityPtr> const& DocumentLayerImpl::allEntities() const {
    return _cadentities;
}

CADEntityPtr DocumentLayerImpl::findByID(ID_DATATYPE id) const {
    if (_cadentities.contains(id)) {
        return _cadentities.value(id);
    }

    throw "Entity not found";
}
