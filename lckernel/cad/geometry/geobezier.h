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

                virtual const std::vector<Coordinate> getCP() const override;

                const Area boundingBox() const override;

                Coordinate nearestPointOnPath(const Coordinate& coord) const override;
                Coordinate nearestPointOnEntity(const Coordinate& coord) const override;

                Coordinate CasteljauAt(std::vector<Coordinate> points, double t) const override;

                Coordinate DirectValueAt(double t) const override;

                const std::vector<Coordinate> Curve(double precession) override;

                const double length() const override;

                const Coordinate tangent(double t) const override;

                const Coordinate normal(double t) const override;

                std::vector<BB_CSPtr> splitHalf() const override;
                BB_CSPtr splitAtT(double t) const override;
                BB_CSPtr offset(const geo::Coordinate& offset) const override;

                BB_CSPtr rotate(const geo::Coordinate& center, double angle) const override;
                BB_CSPtr scale(const geo::Coordinate& center, const geo::Coordinate& factor) const override;
                BB_CSPtr move(const geo::Coordinate& offset) const override;
                BB_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override;

//                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }
            private:

                std::vector<double> nearestPointTValue(const Coordinate &coord) const override;
                const lc::geo::Coordinate returnCasesForNearestPoint(
                        double min_distance, const lc::geo::Coordinate &coord,
                        const Coordinate &ret) const override;

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
