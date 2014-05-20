#ifndef ENTITYCOORDINATE_H
#define ENTITYCOORDINATE_H

#include "cad/geometry/geocoordinate.h"
#include "cad/base/cadentity.h"

namespace lc {
    /**
      * \brief Class that represents a Coordinate associated with a distance to a test point and a identifier for the point
      *
      *
      */
    class EntityCoordinate {
        public:
            /**
              * EntityCoordinate constructor
              * @param geo::Coordinate Coordinate on the entities path
              * @param Distance to a calculated point. This was added to allow sorting of a list of EntityCoordinate.
              * @param unsigned int ID number that identitifes the the coordinate
              */
            EntityCoordinate(const geo::Coordinate& point, double distance, int pointId) : _point(point), _distance(distance), _pointId(pointId) {
            }

            /**
             * @brief EntityCoordinate
             * copy constructor
             * @param ec
             */
            EntityCoordinate(const EntityCoordinate& ec) {
                _point = ec._point;
                _distance = ec._distance;
                _pointId = ec._pointId;
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
}

#endif // ENTITYCOORDINATE_H
