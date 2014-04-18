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

    // TODO: Create a Exists and a Replace
    //throw "Entity not found";
}

void DocumentLayerImpl::replaceEntity(shared_ptr<const CADEntity> tobeRemoved, shared_ptr<const CADEntity> toReplaceWith) {
    if (tobeRemoved->id() == toReplaceWith->id()) {
        throw "tobeRemoved and toReplaceWith must have the same ID";
    }

    if (_cadentities.contains(tobeRemoved->id())) {
        _cadentities.remove(tobeRemoved->id());
        _cadentities.insert(toReplaceWith->id(), toReplaceWith);
        return;
    }

    // TODO: Create a Exists and a Replace
    //throw "Entity not found";
}

QHash<int, shared_ptr<const CADEntity> > DocumentLayerImpl::allEntities() const {
    return _cadentities;
}

shared_ptr<const CADEntity> DocumentLayerImpl::findByID(ID_DATATYPE id) const {
    if (_cadentities.contains(id)) {
        return _cadentities.value(id);
    }

    return shared_ptr<const CADEntity>();
}
