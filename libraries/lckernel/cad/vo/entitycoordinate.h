#ifndef ENTITYCOORDINATE_H
#define ENTITYCOORDINATE_H

#include "cad/geometry/geocoordinate.h"
#include "cad/base/cadentity.h"

namespace lc {
    /**
      * Class that represents a coordinate accosiated with a entity and coordinate id
      * This can be used for example with snappoints where it's needed to know what points
      * of the entity was snapped against.
      *
      */
    class EntityCoordinate {
        public:
            /**
              * EntityCoordinate constructor
              * @param geo::Coordinate Coordinate on the entities path
              * @param unsigned int ID number that identitifes the the coordinate
              */
            EntityCoordinate(const geo::Coordinate& point, double distance, int pointId) : _point(point), _distance(distance), _pointId(pointId) {
            }

            EntityCoordinate& operator = (const EntityCoordinate& ec) {
                if (this != &ec) {
                    _point = ec._point;
                    _distance = ec._distance;
                    _pointId = ec._pointId;
                }

                return *this;
            }

            geo::Coordinate coordinate() const {
                return _point;
            }

            int distance() const {
                return _distance;
            }

            int pointId() const {
                return _pointId;
            }

            static bool sortAscending(const EntityCoordinate& s1 , const EntityCoordinate& s2) {
                return s1.distance() < s2.distance();
            }

        private:
            geo::Coordinate _point;
            double _distance;
            int _pointId;
    };
    typedef shared_ptr<const lc::EntityCoordinate> EntityCoordinatePtr;
}

/**
  * Used for debugging for example qDebug() << "Entity Coordinate:" << entityCoordinate;
  */
//QDebug operator << (QDebug dbg, const lc::EntityCoordinate& c) {
//   dbg.space() << "(" << c.entity()->id() << "," << c.coordinate() << "," << c._point << ")";
//}


#endif // ENTITYCOORDINATE_H
