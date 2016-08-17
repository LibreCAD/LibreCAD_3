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
        class CubicBezier : public BezierBase {
            public:
                /**
                 * Create a new Bezier
                 */
                CubicBezier(const Coordinate& point_a, const Coordinate& point_b, const Coordinate& point_c, const Coordinate& point_d);

                CubicBezier(const CubicBezier &bez);

                virtual const std::vector<Coordinate> getCP() const override;

                virtual const Area boundingBox() const override ;

                virtual Coordinate nearestPointOnPath(const Coordinate& coord) const override;
                virtual Coordinate nearestPointOnEntity(const Coordinate& coord) const override;

                virtual Coordinate CasteljauAt(std::vector<Coordinate> points, double t) const override;

                virtual Coordinate DirectValueAt(double t) const override;

                virtual const std::vector<Coordinate> Curve(double precession) override;

                virtual const double length() const override;

                virtual const Coordinate tangent(double t) const override;

                virtual const Coordinate normal(double t) const override;

                virtual std::vector<BB_CSPtr> splitHalf() const override;
                virtual BB_CSPtr splitAtT(double t) const override;
                virtual BB_CSPtr offset(const geo::Coordinate& offset) const override;

                virtual BB_CSPtr rotate(const geo::Coordinate& center, double angle) const override;
                virtual BB_CSPtr scale(const geo::Coordinate& center, const geo::Coordinate& factor) const override;
                virtual BB_CSPtr move(const geo::Coordinate& offset) const override;
                virtual BB_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override;

            private:

                virtual std::vector<double> nearestPointTValue(const Coordinate &coord) const override;
                virtual const lc::geo::Coordinate returnCasesForNearestPoint(
                        double min_distance, const lc::geo::Coordinate &coord,
                        const Coordinate &ret) const override;
        private:
                Coordinate _pointA;
                Coordinate _pointB;
                Coordinate _pointC;
                Coordinate _pointD;
        };
    }
}
