#ifndef GEOCIRCLE_H
#define GEOCIRCLE_H

#include "cad/const.h"
#include "geocoordinate.h"

namespace lc {
    namespace geo {
        // TODO: Should Circle be a arc with a start angle of 0 and a stop angle of 2PI ?
        class Circle {
            public:
                Circle(const Coordinate& center, double radius);

                const Coordinate& center() const;
                double radius() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

            private:
                const Coordinate _center;
                double _radius;
        };
    }
}

/**
  * Used for debugging for example qDebug() << "my Coordinate:" << point;
  * outputs:
  * My Coordinate:(1,2,3)
  *
  */
QDebug operator << (QDebug dbg, const lc::geo::Circle& c);

#endif // GEOCIRCLE_H
