#include "entitycontainer.h"

#include "cad/base/cadentity.h"

#include <cad/interface/snapable.h>

using namespace lc;

EntityContainer::EntityContainer() {
}

void EntityContainer::insert(CADEntity_CSPtr entity) {
    _cadentities.insert(entity->id(), entity);
}

void EntityContainer::combine(const EntityContainer& entities) {
    for (auto i : entities.allEntities().values()) {
        _cadentities.insert(i->id(), i);
    }
}

void EntityContainer::remove(CADEntity_CSPtr entity) {
    if (_cadentities.contains(entity->id())) {
        _cadentities.remove(entity->id());
        return;
    }
}

QHash<ID_DATATYPE, CADEntity_CSPtr > EntityContainer::allEntities() const {
    return _cadentities;
}

CADEntity_CSPtr EntityContainer::entityByID(ID_DATATYPE id) const {
    return _cadentities.value(id);
}

EntityContainer EntityContainer::entitiesByLayer(const Layer_CSPtr layer) const {
    auto l = layer;
    EntityContainer container;

    for (auto i : _cadentities.values()) {
        if (i->layer() == l) {
            container.insert(i);
        }
    }

    return container;
}

QList<lc::EntityDistance> EntityContainer::getEntitiesNearCoordinate(const lc::geo::Coordinate& point, double distance) const {

    QList<lc::EntityDistance> entities;

    // Now calculate for each entity if we are near the entities path
    for (auto item : _cadentities.values()) {
        Snapable_CSPtr entity = std::dynamic_pointer_cast<const lc::Snapable>(item);

        if (entity != nullptr) { // Not all entities might be snapable, so we only test if this is possible.
            lc::geo::Coordinate eCoordinate = entity->nearestPointOnPath(point);
            lc::geo::Coordinate nearestCoord = eCoordinate - point;

            double cDistance = nearestCoord.magnitude();

            if (cDistance < distance) {
                entities.append(lc::EntityDistance(item, cDistance));
            }
        }
    }

    return entities;
}
