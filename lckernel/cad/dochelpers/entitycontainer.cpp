#include "entitycontainer.h"

#include "cad/base/cadentity.h"

#include <cad/interface/snapable.h>

using namespace lc;

EntityContainer::EntityContainer() {
    _tree = new QuadTree(geo::Area(geo::Coordinate(-10000.,-10000.), geo::Coordinate(10000.,10000.)));
}

EntityContainer::EntityContainer(const EntityContainer &other) {
     _tree = new QuadTree(*other._tree);
}

EntityContainer::~EntityContainer() {
    delete _tree;
}



void EntityContainer::insert(CADEntity_CSPtr entity) {
//    _cadentities.insert(std::make_pair(entity->id(), entity));
    _tree->insert(entity);
}

void EntityContainer::combine(const EntityContainer& entities) {
    for (auto i : entities.allEntities()) {
//        _cadentities.insert(std::make_pair(i->id(), i));
        _tree->insert(i);
    }
}

void EntityContainer::remove(CADEntity_CSPtr entity) {
//    _cadentities.erase(entity->id());
    _tree->erase(entity);
}

/*std::map<ID_DATATYPE, CADEntity_CSPtr> EntityContainer::allEntities() const {
    return _cadentities;
}*/

std::vector<CADEntity_CSPtr> EntityContainer::allEntities() const {
/*    std::vector<CADEntity_CSPtr> v;
    for (auto item : _cadentities) {
        v.push_back(item.second);
    } */

    std::vector<CADEntity_CSPtr> v = _tree->retrieve();
    return v;
}

CADEntity_CSPtr EntityContainer::entityByID(ID_DATATYPE id) const {
    /*
    if (_cadentities.count(id) > 0) {
        return _cadentities.at(id);
    } */

    return _tree->entityByID(id);
}

EntityContainer EntityContainer::entitiesByLayer(const Layer_CSPtr layer) const {
    EntityContainer container;
/*    auto l = layer;

    for (auto i : _cadentities) {
        if (i.second->layer() == l) {
            container.insert(i.second);
        }
    }*/

    for (auto i : allEntities()) {
        if (i->layer() == layer) {
            container.insert(i);
        }
    }

    return container;
}

EntityContainer EntityContainer::entitiesByArea(const geo::Area & area) const {
    EntityContainer container;
    std::vector<CADEntity_CSPtr> entities = _tree->retrieve(area);
    for (auto i : entities) {
            container.insert(i);
    }
    return container;
}

std::vector<lc::EntityDistance> EntityContainer::getEntitiesNearCoordinate(const lc::geo::Coordinate& point, double distance) const {

    std::vector<lc::EntityDistance> entities;

    // Now calculate for each entity if we are near the entities path
    /*
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
    } */

    return entities;
}

void EntityContainer::optimise() {
    _tree->optimise();
}
