#include "entitycontainer.h"

using namespace lc;

EntityContainer::EntityContainer() {
}

void EntityContainer::insert(shared_ptr<const CADEntity> entity) {
    _cadentities.insert(entity->id(), entity);
}

void EntityContainer::combine(const EntityContainer& entities) {
    QHash<ID_DATATYPE, shared_ptr<const CADEntity>>::const_iterator i = entities.allEntities().constBegin();

    while (i != _cadentities.constEnd()) {
        _cadentities.insert(i.key(), i.value());
    }
}

void EntityContainer::remove(shared_ptr<const CADEntity> entity) {
    if (_cadentities.contains(entity->id())) {
        _cadentities.remove(entity->id());
        return;
    }
}

QHash<ID_DATATYPE, shared_ptr<const CADEntity> > EntityContainer::allEntities() const {
    return _cadentities;
}

shared_ptr<const CADEntity> EntityContainer::entityByID(ID_DATATYPE id) const {
    return _cadentities.value(id);
}

EntityContainer EntityContainer::entitiesByLayer(const shared_ptr<const Layer> layer) const {

    auto l = layer;
    QHash<ID_DATATYPE, shared_ptr<const CADEntity>>::const_iterator i = _cadentities.constBegin();

    EntityContainer container;

    while (i != _cadentities.constEnd()) {
        if (i.value()->layer() == l) {
            container.insert(i.value());
        }

        ++i;
    }

    return container;
}
