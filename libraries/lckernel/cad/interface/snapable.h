#ifndef SNAPABLE_H
#define SNAPABLE_H

#include <QList>

#include "cad/geometry/geocoordinate.h"

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
              */
            virtual QList<geo::Coordinate> snapPoints(const geo::Coordinate& mousePointer, double minDistanceToSnap, int maxNumberOfSnapPoints) const = 0;
    };
    typedef shared_ptr<const lc::Snapable> SnapablePtr;
}

#endif // SNAPABLE_H
