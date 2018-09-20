#pragma once

#include "cad/base/cadentity.h"

namespace lc {
    /**
      * Entity Distance is a value object that can be used to associate a entity with a specific distance
      * Usually distances comes from a calculation between a coordinate and this entity
      *
      */
    class EntityDistance {
    public:
        /**
          * EntityDistance constructor
          * @param CADEntity_CSPtr Entity in question
          * @param geo::Coordinate Coordinate on the entities path
          * @param unsigned int ID number that identitifes the coordinate
          *
          */
        EntityDistance(entity::CADEntity_CSPtr cadEntity, const geo::Coordinate &coordinate) : _cadEntity(cadEntity),
                                                                                               _coordinate(coordinate) {

        }

        entity::CADEntity_CSPtr entity() const {
            return _cadEntity;
        }

        geo::Coordinate coordinate() const {
            return _coordinate;
        }

    private:
        entity::CADEntity_CSPtr _cadEntity;
        geo::Coordinate _coordinate;
    };

    struct EntityDistanceSorter {
        EntityDistanceSorter(const geo::Coordinate &reference) : _reference(reference) { }

        bool operator()(const EntityDistance &i1, const EntityDistance &i2) {
            const auto d1 = i1.coordinate().distanceTo(_reference);
            const auto d2 = i2.coordinate().distanceTo(_reference);
            return d1 < d2;
        }

        geo::Coordinate _reference;
    };
}
