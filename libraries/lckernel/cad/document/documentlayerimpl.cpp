#include "documentlayerimpl.h"

using namespace lc;

DocumentLayerImpl::DocumentLayerImpl() {
    _layer = NULL;
    _cadentities = NULL;
}
DocumentLayerImpl::DocumentLayerImpl(Layer* layer) {
    _layer = layer;
    _cadentities = new QVector<CADEntity*>();
}

DocumentLayerImpl::~DocumentLayerImpl() {
    qDeleteAll(*_cadentities);
    delete _cadentities;
}

Layer* DocumentLayerImpl::layer() const {
    return this->_layer;
}

void DocumentLayerImpl::addEntity(CADEntity* entity) {
    _cadentities->append(entity);
}

void DocumentLayerImpl::removeEntity(ID_DATATYPE id) {
    long unsigned int size = _cadentities->size();

    for (long unsigned int idx = 0; idx < size; ++idx) {
        if (_cadentities->at(idx)->id() == id) {
            delete _cadentities->value(idx);
            _cadentities->remove(idx);
            return;
        }
    }

    throw "Entity not found";
}
