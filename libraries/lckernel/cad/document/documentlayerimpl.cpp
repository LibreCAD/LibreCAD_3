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

/**
  * Add's a CAD entity to this layer
  */
void DocumentLayerImpl::addEntity(CADEntity* entity) {
    _cadentities->append(entity);
}
