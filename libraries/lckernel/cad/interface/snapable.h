#ifndef SNAPABLE_H
#define SNAPABLE_H

#include <QList>
#include "cad/geometry/geocoordinate.h"
#include "cad/vo/entitycoordinate.h"

namespace lc {
    /**
      * Any entity that we can snap against needs to implement the snapable interface
      *
      */
    class Snapable {
        public:
            /**
              * This function returns a number of snap points up to a maximum number of snappoints ordered
              * by distance from mousePointer
              * We might need to decide to return Objects that can hold additional information then just
              * a snapped point
              * @param
              */
            virtual QList<lc::EntityCoordinate> snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const = 0;

            /**
             * Find the closest point on the entity from the given coordinate
             *
             * @param coord Coordinate from where to lookup the point
             * @param bool  when !=null this boolean we show true if the coordinate is on the entities path
             * @return Coordinate on entity that's closest to coord
             */
            virtual geo::Coordinate nearestPointOnEntity(const geo::Coordinate& coord) const = 0;

    };
    typedef shared_ptr<const lc::Snapable> SnapablePtr;
}

#endif // SNAPABLE_H


/**
 * Must be overwritten to get the closest coordinate to the
* given coordinate which is on this entity.
 *
 * @param coord Coordinate (typically a mouse coordinate)
 * @param dist Pointer to a value which will contain the measured
 * distance between \p coord and the point. The passed pointer can
 * also be \p NULL in which case the distance will be lost.
 *
 * @return The closest coordinate.
 */
