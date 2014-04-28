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

QList<shared_ptr<const CADEntity> > EntityContainer::allEntities() const {
    return _cadentities.values();
}

shared_ptr<const CADEntity> EntityContainer::findEntityByID(ID_DATATYPE id) const {
    return _cadentities.value(id);
}

EntityContainer EntityContainer::findEntitiesByLayer(const shared_ptr<const Layer> layer) const {
    EntityContainer container;

    QHash<ID_DATATYPE, shared_ptr<const CADEntity>>::const_iterator i = _cadentities.constBegin();
    while (i != _cadentities.constEnd()) {
        if (i.value()->layer() == layer) {
            container.addEntity(i.value());
        }
    }
    return container;
}
