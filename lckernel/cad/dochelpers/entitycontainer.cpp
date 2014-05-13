#include "entitycontainer.h"

#include "cad/base/cadentity.h"

using namespace lc;

EntityContainer::EntityContainer() {
}

void EntityContainer::insert(std::shared_ptr<const CADEntity> entity) {
    _cadentities.insert(entity->id(), entity);
}

void EntityContainer::combine(const EntityContainer& entities) {
    foreach(auto i, entities.allEntities().values()) {
        _cadentities.insert(i->id(), i);
    }
}

void EntityContainer::remove(std::shared_ptr<const CADEntity> entity) {
    if (_cadentities.contains(entity->id())) {
        _cadentities.remove(entity->id());
        return;
    }
}

QHash<ID_DATATYPE, std::shared_ptr<const CADEntity> > EntityContainer::allEntities() const {
    return _cadentities;
}

std::shared_ptr<const CADEntity> EntityContainer::entityByID(ID_DATATYPE id) const {
    return _cadentities.value(id);
}

EntityContainer EntityContainer::entitiesByLayer(const std::shared_ptr<const Layer> layer) const {
    auto l = layer;
    EntityContainer container;
    foreach(auto i, _cadentities.values()) {
        if (i->layer() == l) {
            container.insert(i);
        }
    }
    return container;
}
