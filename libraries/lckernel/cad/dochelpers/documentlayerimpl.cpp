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
    _cadentities.append(entity);
}

void DocumentLayerImpl::removeEntity(ID_DATATYPE id) {
    long unsigned int size = _cadentities.size();

    for (long unsigned int idx = 0; idx < size; ++idx) {
        if (_cadentities.at(idx)->id() == id) {
            _cadentities.removeAt(idx);
            return;
        }
    }

    throw "Entity not found";
}

QList<CADEntityPtr> const& DocumentLayerImpl::allEntities() const {
    return _cadentities;
}

CADEntityPtr DocumentLayerImpl::findByID(ID_DATATYPE id) const {
    long unsigned int size = _cadentities.size();

    for (long unsigned int idx = 0; idx < size; ++idx) {
        if (_cadentities.at(idx)->id() == id) {
            return _cadentities.at(idx);
        }
    }

    throw "Entity not found";
}
