#ifndef ENTITYDISTANCE_H
#define ENTITYDISTANCE_H

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
              * @param CADEntityPtr Entity in question
              * @param geo::Coordinate Coordinate on the entities path
              * @param unsigned int ID number that identitifes the the coordinate
              */
            EntityDistance(CADEntityPtr cadEntity, double distance) : _cadEntity(cadEntity), _distance(distance) {

            }

            CADEntityPtr entity() const {
                return _cadEntity;
            }

            int distance() const {
                return _distance;
            }

            static bool sortAscending(const EntityDistance& s1 , const EntityDistance& s2) {
                return s1.distance() < s2.distance();
            }

        private:
            CADEntityPtr _cadEntity;
            double _distance;
    };
    typedef shared_ptr<const lc::EntityDistance> EntityDistancePtr;
}

#endif // ENTITYDISTANCE_H
