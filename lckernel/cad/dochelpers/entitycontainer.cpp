#include "entitycontainer.h"

#include "cad/base/cadentity.h"

#include <cad/interface/snapable.h>

using namespace lc;

EntityContainer::EntityContainer() {
}

void EntityContainer::insert(CADEntity_CSPtr entity) {
    _cadentities.insert(std::make_pair(entity->id(), entity));
}

void EntityContainer::combine(const EntityContainer& entities) {
    for (auto i : entities.allEntities()) {
        _cadentities.insert(i);
    }
}

void EntityContainer::remove(CADEntity_CSPtr entity) {
    _cadentities.erase(entity->id());
}

std::map<ID_DATATYPE, CADEntity_CSPtr> EntityContainer::allEntities() const {
    return _cadentities;
}

std::vector<CADEntity_CSPtr> EntityContainer::allEntitiesAsVector() const {
    std::vector<CADEntity_CSPtr> v;

    for (auto item : _cadentities) {
        v.push_back(item.second);
    }

    return v;
}

CADEntity_CSPtr EntityContainer::entityByID(ID_DATATYPE id) const {
    if (_cadentities.count(id) > 0) {
        return _cadentities.at(id);
    }

    return CADEntity_CSPtr();
}

EntityContainer EntityContainer::entitiesByLayer(const Layer_CSPtr layer) const {
    auto l = layer;
    EntityContainer container;

    for (auto i : _cadentities) {
        if (i.second->layer() == l) {
            container.insert(i.second);
        }
    }

    return container;
}

std::vector<lc::EntityDistance> EntityContainer::getEntitiesNearCoordinate(const lc::geo::Coordinate& point, double distance) const {

    std::vector<lc::EntityDistance> entities;

    // Now calculate for each entity if we are near the entities path
    for (auto item : _cadentities) {
        Snapable_CSPtr entity = std::dynamic_pointer_cast<const lc::Snapable>(item.second);

        if (entity != nullptr) { // Not all entities might be snapable, so we only test if this is possible.
            lc::geo::Coordinate eCoordinate = entity->nearestPointOnPath(point);
            lc::geo::Coordinate nearestCoord = eCoordinate - point;

            double cDistance = nearestCoord.magnitude();

            if (cDistance < distance) {
                entities.push_back(lc::EntityDistance(item.second, cDistance));
            }
        }
    }

    return entities;
}
