#ifndef GEOVECTOR_H
#define GEOVECTOR_H

#include "cad/const.h"
#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Vector  {
            public:
                // TODO: Test for validity?
                explicit Vector(const Coordinate& start, const Coordinate& end) : _start(start), _end(end) {}

                // TODO: Test for validity?
                Vector(const Vector& v) : _start(v._start), _end(v._end)  {}

                inline const Coordinate start() const {
                    return _start;
                }
                inline const Coordinate end() const {
                    return _end;
                }

                inline Vector& operator = (const Vector& v) {
                    if (this != &v) {
                        _start = v._start;
                        _end = v._end;
                    }

                    return *this;
                }

                inline Coordinate nearestPointOnPath(const Coordinate& coord) const {
                    Coordinate direction = this->end() - this->start();
                    Coordinate vpc = coord - this->start();
                    vpc = start() + direction * Coordinate(vpc).dot(direction) / direction.squared();
                    return vpc;
                }

                /*
                inline bool isCoordinateOnPath(const Coordinate& coord) const {
                    geo::Area area(_start, _end);
                    if (area.inArea(coord) && ((nearestPointOnPath(coord) - coord).squared() < 1.0e-8)) {
                        return true;
                    }

                    return false;
                }*/

                inline bool isCoordinateOnPath(const Coordinate& coord) const {
                    geo::Coordinate minP(Coordinate(std::min(_start.x(), _end.x()), std::min(_start.y(), _end.y())));
                    geo::Coordinate maxP(Coordinate(std::max(_start.x(), _end.x()), std::max(_start.y(), _end.y())));

                    bool inArea = (coord.x() >= minP.x() && coord.x() <= maxP.x() && coord.y() >= minP.y() && coord.y() <= maxP.y());

                    if (inArea && ((nearestPointOnPath(coord) - coord).magnitude() < 1.0e-4)) {
                        return true;
                    }
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
