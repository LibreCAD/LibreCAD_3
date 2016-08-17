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
    class BezierBase;
    using BB_CSPtr = std::shared_ptr<const BezierBase>;
    using BB_SPtr = std::shared_ptr<BezierBase>;

    class BezierBase  : public Base, virtual public Visitable {
            public:

                BezierBase() {}
                virtual ~BezierBase() {}

                virtual const Area boundingBox() const = 0;

                virtual Coordinate nearestPointOnPath(const Coordinate& coord) const = 0;
                virtual Coordinate nearestPointOnEntity(const Coordinate& coord) const = 0;

                virtual Coordinate CasteljauAt(std::vector<Coordinate> points, double t) const = 0;

                virtual const std::vector<Coordinate> getCP() const = 0;

                virtual Coordinate DirectValueAt(double t) const = 0;

                virtual const std::vector<Coordinate> Curve(double precession) = 0;

                virtual const double length() const = 0;

                virtual const Coordinate tangent(double t) const = 0;

                virtual const Coordinate normal(double t) const = 0;

                virtual std::vector<BB_CSPtr> splitHalf() const = 0;
                virtual BB_CSPtr splitAtT(double t) const = 0;
                virtual BB_CSPtr offset(const geo::Coordinate& offset) const = 0;

                virtual BB_CSPtr rotate(const geo::Coordinate& center, double angle) const = 0;
                virtual BB_CSPtr scale(const geo::Coordinate& center, const geo::Coordinate& factor) const = 0;
                virtual BB_CSPtr move(const geo::Coordinate& offset) const = 0;
                virtual BB_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const = 0;

                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

                virtual std::vector<double> nearestPointTValue(const Coordinate &coord) const = 0;
                virtual const lc::geo::Coordinate returnCasesForNearestPoint(
                        double min_distance, const lc::geo::Coordinate &coord,
                        const Coordinate &ret) const = 0;
        };
    }
}
