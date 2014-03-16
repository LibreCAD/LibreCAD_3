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
            /*!
             * \brief Find a number of snap points the line has available
             * This function returns a ordered list, closest to \em coord and can return multiple snap points
             *
             * \param coord Coordinate to lookup the nearest coordinate from
             * \param minDistanceToSnap Minimum distance to the path to
             * \param maxNumberOfSnapPoints Maximum number of snappoints that have to be looked up
             *
             * \sa lc::EntityCoordinate
             */
            virtual QList<lc::EntityCoordinate> snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const = 0;

            /*!
             * \brief Find the nearest point on the path for this entity for the coordinate \em coord
             *
             * \param coord Coordinate to lookup the nearest coordinate from
             * \sa lc::CADEntity
             */
            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const = 0;
    };
}

#endif // SNAPABLE_H


