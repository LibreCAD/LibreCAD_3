#pragma once

#include "cad/geometry/geocoordinate.h"
#include "cad/math/equation.h"
#include <Eigen/Dense>
#include "cad/geometry/geobezierbase.h"
#include "cad/geometry/geobezier.h"
#include "cad/geometry/geobeziercubic.h"
#include "cad/geometry/geovector.h"
#include "cad/geometry/geocircle.h"
#include "cad/geometry/geoarc.h"
#include "cad/geometry/geoellipse.h"
#include "cad/geometry/geospline.h"
#include "cad/math/helpermethods.h"

namespace lc {
namespace maths {
class Intersection {
public:
    static std::vector<lc::geo::Coordinate> LineLine(const Equation& l1, const Equation& l2);
    static std::vector<lc::geo::Coordinate> LineQuad(const Equation& l1, const Equation& q1);
    static std::vector<geo::Coordinate> QuadQuad(const Equation& l1, const Equation& l2);
    static std::vector<geo::Coordinate> bezierLine(const geo::BB_CSPtr& B, const geo::Vector& V);
    static std::vector<geo::Coordinate> bezierCircle(const geo::BB_CSPtr& B, const geo::Circle& C);
    static std::vector<geo::Coordinate> bezierArc(const geo::BB_CSPtr& B, const geo::Arc& A);
    static std::vector<geo::Coordinate> bezierEllipse(const geo::BB_CSPtr& B, const geo::Ellipse& E);
    static std::vector<geo::Coordinate> bezierBezier(const geo::BB_CSPtr& B1, const geo::BB_CSPtr& B2);

    static std::vector<geo::Coordinate> splineLine(const geo::Spline& B, const geo::Vector& V);
    static std::vector<geo::Coordinate> splineCircle(const geo::Spline& B, const geo::Circle& C);
    static std::vector<geo::Coordinate> splineArc(const geo::Spline& B, const geo::Arc& A);
    static std::vector<geo::Coordinate> splineEllipse(const geo::Spline& B, const geo::Ellipse& E);
    static std::vector<geo::Coordinate> splineBezier(const geo::Spline& B1, const geo::BB_CSPtr& B2);
    static std::vector<geo::Coordinate> splineSpline(const geo::Spline& B1, const geo::Spline& B2);
    static std::vector<geo::Coordinate> bezCircleIntersect(const lc::geo::BB_CSPtr& bez,
            const geo::Coordinate& ec,
            double rx, double ry);
private:
    static void bezBez(const geo::BB_CSPtr& B1, const geo::BB_CSPtr& B2, std::vector<geo::Coordinate>&ret);
};
}
}
