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

            protected:
                void start(const Coordinate& coord);
                void end(const Coordinate& coord);

            private:
                Coordinate _start;
                Coordinate _end;

        };
    }
}

#endif // GEOVECTOR_H
