#pragma once

#include "cad/const.h"
#include "geoarea.h"
#include "geocoordinate.h"
#include "geobase.h"
#include <cad/math/lcmath.h>
#include <cmath>
#include <cstdlib>
#include <vector>

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

                const double length() const;

                const Coordinate tangent(double t) const;

                const Coordinate normal(double t) const;

                Bezier rotate(const geo::Coordinate& center, double angle);
                Bezier scale(const geo::Coordinate& center, const geo::Coordinate& factor) const;
                Bezier move(const geo::Coordinate& offset) const;
                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }
            private:

                std::vector<double> nearestPointTValue(const Coordinate &coord) const;
                const lc::geo::Coordinate returnCasesForNearestPoint(
                        double min_distance, const lc::geo::Coordinate &coord,
                        const Coordinate &ret) const;

                friend std::ostream& operator<<(std::ostream& os, const Bezier& bez) {
                    os << "Bezier(A=" << bez._pointA << " radius=" << bez._pointB << " startAngle=" << bez._pointC << ")";
                    return os;
                }

        private:
                Coordinate _pointA;
                Coordinate _pointB;
                Coordinate _pointC;
        };
    }
}
