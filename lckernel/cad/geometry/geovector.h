#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include "geobase.h"
#include "cad/math/quadratic_math.h"
#include <vector>
#include "cad/base/visitor.h"

namespace lc {
    namespace geo {
        class Vector : public Base, virtual public Visitable {
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

                /**
                 * Return the neatrest point on the entities path
                 */
                inline Coordinate nearestPointOnPath(const Coordinate& coord) const {
                    Coordinate direction = this->end() - this->start();
                    Coordinate vpc = coord - this->start();
                    vpc = start() + direction * Coordinate(vpc).dot(direction) / direction.squared();
                    return vpc;
                }

                /**
                 * Return the nearest point on the entity
                 */
                inline Coordinate nearestPointOnEntity(const Coordinate& coord) const {
                    const auto vpc = nearestPointOnPath(coord);
                    // Note, I could use Area for this but didn't want that dependency here
                    const geo::Coordinate minP(Coordinate(std::min(_start.x()-LCTOLERANCE, _end.x()-LCTOLERANCE), std::min(_start.y()-LCTOLERANCE, _end.y()-LCTOLERANCE)));
                    const geo::Coordinate maxP(Coordinate(std::max(_start.x()+LCTOLERANCE, _end.x()+LCTOLERANCE), std::max(_start.y()+LCTOLERANCE, _end.y()+LCTOLERANCE)));

                    bool inArea = (vpc.x() >= minP.x() && vpc.x() <= maxP.x() && vpc.y() >= minP.y() && vpc.y() <= maxP.y());
                    if (inArea) {
                        return vpc;
                    } else {
                        const double dStart = coord.distanceTo(start());
                        const double dEnd = coord.distanceTo(end());
                        return dStart < dEnd?start():end();
                    }
                }
                /*
                inline bool isCoordinateOnPath(const Coordinate& coord) const {
                    geo::Area area(_start, _end);
                    if (area.inArea(coord) && ((nearestPointOnPath(coord) - coord).squared() < 1.0e-8)) {
                        return true;
                    }

                    return false;
                }*/

                /**
                 * Use nearestPointOnEntity or nearestPointOnPath and test for the distance
                inline bool isCoordinateOnPath(const Coordinate& coord) const {

                    geo::Coordinate minP(Coordinate(std::min(_start.x()-LCTOLERANCE, _end.x()-LCTOLERANCE), std::min(_start.y()-LCTOLERANCE, _end.y()-LCTOLERANCE)));
                    geo::Coordinate maxP(Coordinate(std::max(_start.x()+LCTOLERANCE, _end.x()+LCTOLERANCE), std::max(_start.y()+LCTOLERANCE, _end.y()+LCTOLERANCE)));

                    // bool inArea = (coord.x() >= minP.x() && coord.x() <= maxP.x() && coord.y() >= minP.y() && coord.y() <= maxP.y());

                    // We might need a other cunction called 'isCoordinateOnEntity' and we use the below condition
                    //if (inArea && ((nearestPointOnPath(coord) - coord).magnitude() < LCTOLERANCE)) {
                    //    return true;
                    //}


                    if ((nearestPointOnPath(coord) - coord).magnitude() < LCTOLERANCE) {
                        return true;
                    }

                    return false;
                }                 */


            /**
                 * @return The angle of the line (from start to endpoint).
                 */
                double Angle1() const {
                    return _start.angleTo(_end);
                }

                /**
                 * @return The angle of the line (from end to startpoint).
                 */
                double Angle2() const {
                    return _end.angleTo(_start);
                }

                /**
                 * @brief quadratic, Returns quadratic for the entity
                 * @return Quadratic quadratic equation
                 */
                Quadratic quadratic() const {
                    std::vector<double> ce(3, 0.);
                    auto&& dvp = this->end() - this->start();
                    geo::Coordinate normal(-dvp.y(), dvp.x());
                    ce[0] = normal.x();
                    ce[1] = normal.y();
                    ce[2] = - normal.dot(this->end());
                    return Quadratic(ce);
                }

                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            private:
                friend std::ostream& operator<<(std::ostream& os, const Vector& e) {
                    os << "Vector(start=" << e._start << " end=" << e._end << ")";
                    return os;
                }
            private:
                 Coordinate _start;
                 Coordinate _end;

        };
    }
}

// GEOVECTOR_H
