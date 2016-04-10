#pragma once

#include "cad/const.h"
#include "geoarea.h"
#include "geocoordinate.h"
#include "geobase.h"
#include <vector>
#include "cad/math/quadratic_math.h"


namespace lc {
    namespace geo {
        class Bezier  : public Base, virtual public Visitable {
            public:
                /**
                 * Create a new Bezier
                 */
                Bezier(const Coordinate& point_a, const Coordinate& point_b, const Coordinate& point_c);

                Bezier(const Bezier &bez);

                const Coordinate pointA() const;

                const Coordinate pointB() const;

                const Coordinate pointC() const;

                const Area boundingBox() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                Coordinate nearestPointOnEntity(const Coordinate& coord) const;

                Coordinate CasteljauAt(std::vector<Coordinate> points, double t) const;

                Coordinate DirectValueAt(double t) const;

                const std::vector<Coordinate> Curve(double precession);

                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }
            private:

                friend std::ostream& operator<<(std::ostream& os, const Bezier& bez) {
                    os << "Bezier(A=" << bez._pointA << " radius=" << bez._pointA << " startAngle=" << bez._pointA << ")";
                    return os;
                }

        private:
                Coordinate _pointA;
                Coordinate _pointB;
                Coordinate _pointC;
        };
    }
}
