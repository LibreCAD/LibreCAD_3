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
     * Create a new CUBIC Bezier
     */
    CubicBezier(Coordinate point_a, Coordinate point_b, Coordinate point_c, Coordinate point_d);
    CubicBezier(const CubicBezier &bez);

private:
    Coordinate _pointA;
    Coordinate _pointB;
    Coordinate _pointC;
    Coordinate _pointD;

public:
    //Methods from BezierBase
    const std::vector<Coordinate> getCP() const override;
    const Area boundingBox() const override ;
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
    std::vector<double> nearestPointTValue(const Coordinate &coord) const override;
    const lc::geo::Coordinate returnCasesForNearestPoint(
        double min_distance, const lc::geo::Coordinate &coord,
        const Coordinate &ret) const override;
};
}
}
