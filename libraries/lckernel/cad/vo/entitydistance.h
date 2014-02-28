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
              * @param boost::shared_ptr<const lc::CADEntity> Entity in question
              * @param geo::Coordinate Coordinate on the entities path
              * @param unsigned int ID number that identitifes the the coordinate
              *
              */
            EntityDistance(boost::shared_ptr<const lc::CADEntity> cadEntity, double distance) : _cadEntity(cadEntity), _distance(distance) {

            }

            boost::shared_ptr<const lc::CADEntity> entity() const {
                return _cadEntity;
            }

            int distance() const {
                return _distance;
            }

            static bool sortAscending(const EntityDistance& s1 , const EntityDistance& s2) {
                return s1.distance() < s2.distance();
            }

        private:
            boost::shared_ptr<const lc::CADEntity> _cadEntity;
            double _distance;
    };
}

#endif // ENTITYDISTANCE_H
