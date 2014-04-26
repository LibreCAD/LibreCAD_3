#include "entitycontainer.h"

using namespace lc;

EntityContainer::EntityContainer() {
}

void EntityContainer::addEntity(shared_ptr<const CADEntity> entity) {
    _cadentities.insert(entity->id(), entity);
}

void EntityContainer::removeEntity(shared_ptr<const CADEntity> entity) {
    if (_cadentities.contains(entity->id())) {
        _cadentities.remove(entity->id());
        return;
    }
}

void EntityContainer::replaceEntity(shared_ptr<const CADEntity> toReplaceWith) {

    if (_cadentities.contains(toReplaceWith->id())) {
        _cadentities.remove(toReplaceWith->id());
        _cadentities.insert(toReplaceWith->id(), toReplaceWith);
        return;
    }
}

QHash<int, shared_ptr<const CADEntity> > EntityContainer::allEntities() const {
    return _cadentities;
}

shared_ptr<const CADEntity> EntityContainer::findEntityByID(ID_DATATYPE id) const {
    return _cadentities.value(id);
}
