#pragma once

#include "cad/base/cadentity.h"

namespace lc {
    /**
      * Entity Distance is a value object that can be used to associate a entity with a specific distance
      * Ususally distances comes from a calculation between a coordinate and this entity
      *
      */
    class EntityDistance {
        public:
            /**
              * EntityDistance constructor
              * @param CADEntity_CSPtr Entity in question
              * @param geo::Coordinate Coordinate on the entities path
              * @param unsigned int ID number that identitifes the the coordinate
              *
              */
            EntityDistance(entity::CADEntity_CSPtr cadEntity, const geo::Coordinate &coordinate, double distance) : _cadEntity(cadEntity), _coordinate(coordinate), _distance(distance) {

            }

            entity::CADEntity_CSPtr entity() const {
                return _cadEntity;
            }

            int distance() const {
                return _distance;
            }

            geo::Coordinate coordinate() const {
                return _coordinate;
            }

            static bool sortAscending(const EntityDistance& s1 , const EntityDistance& s2) {
                return s1.distance() < s2.distance();
            }

        private:
            entity::CADEntity_CSPtr _cadEntity;
             geo::Coordinate _coordinate;
             double _distance;
    };
}
