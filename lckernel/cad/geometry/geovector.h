#ifndef GEOVECTOR_H
#define GEOVECTOR_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Vector  {
            public:
                Vector(const Coordinate& start, const Coordinate& end);
                Vector(const Vector& v);

                const Coordinate& start() const;
                const Coordinate& end() const;

                Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

            private:
                const Coordinate _start;
                const Coordinate _end;

        };
    }
}


/**
  * Used for debugging for example qDebug() << "my Coordinate:" << point;
  * outputs:
  * My Coordinate:(1,2,3)
  *
  */
QDebug operator << (QDebug dbg, const lc::geo::Vector& c);

#endif // GEOVECTOR_H
