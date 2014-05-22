#ifndef GEOVECTOR_H
#define GEOVECTOR_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Area;
        class Vector  {
            public:
                // TODO: Test for validity?
                explicit Vector(const Coordinate& start, const Coordinate& end) : _start(start), _end(end) {}

                // TODO: Test for validity?
                explicit Vector(const Vector& v) : _start(v._start), _end(v._end)  {}

                inline const Coordinate start() const {
                    return _start;
                }
                inline const Coordinate end() const {
                    return _end;
                }

                inline Coordinate nearestPointOnPath(const Coordinate& coord) const {
                    Coordinate direction = this->end() - this->start();
                    Coordinate vpc = coord - this->start();
                    vpc = start() + direction * Coordinate(vpc).dot(direction) / direction.squared();
                    return vpc;
                }

                inline bool isCoordinateOnPath(const Coordinate& coord) const {
                    return (nearestPointOnPath(coord) - coord).magnitude() < 1.0e-4;
                }


            protected:
                inline void start(const Coordinate& coord) {
                    _start = coord;
                }
                inline void end(const Coordinate& coord) {
                    _end = coord;
                }

            private:
                Coordinate _start;
                Coordinate _end;

        };
    }
}

#endif // GEOVECTOR_H
