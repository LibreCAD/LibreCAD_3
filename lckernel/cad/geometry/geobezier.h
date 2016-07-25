#pragma once

#include "cad/const.h"
#include "geoarea.h"
#include "geocoordinate.h"
#include "geobase.h"
#include <cad/math/lcmath.h>
#include <cmath>
#include <cstdlib>
#include <vector>
#include "geobezierbase.h"

namespace lc {
    namespace geo {
        class Bezier  : public BezierBase {
            public:
                /**
                 * Create a new Bezier
                 */
                Bezier(const Coordinate& point_a, const Coordinate& point_b, const Coordinate& point_c);

                Bezier(const Bezier &bez);

                std::vector<Coordinate> getCP() {
                    std::vector<Coordinate> ret{_pointA, _pointB, _pointC};
                    return ret;
                }
                const Area boundingBox() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                Coordinate nearestPointOnEntity(const Coordinate& coord) const;

                Coordinate CasteljauAt(std::vector<Coordinate> points, double t) const;

                Coordinate DirectValueAt(double t) const;

                const std::vector<Coordinate> Curve(double precession);

                const double length() const;

                const Coordinate tangent(double t) const;

                const Coordinate normal(double t) const;

                std::vector<BB_CSPtr> splitHalf() const;

                BB_CSPtr offset(const geo::Coordinate& offset) const;

                BB_CSPtr rotate(const geo::Coordinate& center, double angle) const;
                BB_CSPtr scale(const geo::Coordinate& center, const geo::Coordinate& factor) const;
                BB_CSPtr move(const geo::Coordinate& offset) const;
                BB_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const;

//                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }
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
