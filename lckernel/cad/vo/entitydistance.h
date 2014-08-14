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
            EntityDistance(CADEntity_CSPtr cadEntity, double distance) : _cadEntity(cadEntity), _distance(distance) {

            }

            CADEntity_CSPtr entity() const {
                return _cadEntity;
            }

            int distance() const {
                return _distance;
            }

            static bool sortAscending(const EntityDistance& s1 , const EntityDistance& s2) {
                return s1.distance() < s2.distance();
            }

        private:
            CADEntity_CSPtr _cadEntity;
            double _distance;
    };
}
