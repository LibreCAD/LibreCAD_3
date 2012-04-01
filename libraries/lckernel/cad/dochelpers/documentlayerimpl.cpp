#include "documentlayerimpl.h"

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

void DocumentLayerImpl::addEntity(CADEntity* entity) {
    _cadentities.append(entity);
}

void DocumentLayerImpl::removeEntity(ID_DATATYPE id) {
    long unsigned int size = _cadentities.size();

    for (long unsigned int idx = 0; idx < size; ++idx) {
        if (_cadentities.at(idx)->id() == id) {
            delete _cadentities.value(idx);
            _cadentities.remove(idx);
            return;
        }
    }

    throw "Entity not found";
}

QVector<CADEntity*> DocumentLayerImpl::allEntities() const {
    return _cadentities;
}

CADEntity* DocumentLayerImpl::findByID(ID_DATATYPE id) const {
    long unsigned int size = _cadentities.size();

    for (long unsigned int idx = 0; idx < size; ++idx) {
        if (_cadentities.at(idx)->id() == id) {
            return _cadentities.at(idx);
        }
    }

    throw "Entity not found";
}
