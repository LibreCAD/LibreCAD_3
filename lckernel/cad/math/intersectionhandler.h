#pragma once

#include "cad/geometry/geocoordinate.h"
#include "cad/math/equation.h"
#include <Eigen/Dense>
#include "cad/geometry/geobezier.h"
#include "cad/geometry/geovector.h"
#include "cad/geometry/geocircle.h"
#include "cad/geometry/geoarc.h"
#include "cad/geometry/geoellipse.h"

namespace lc {
    namespace maths {
        class Intersection {
        public:
        static std::vector<lc::geo::Coordinate> LineLine(
            const Equation& l1, const Equation& l2);
        static std::vector<lc::geo::Coordinate> LineQuad(
            const Equation& l1, const Equation& q1);
        static std::vector<geo::Coordinate> QuadQuad(
            const Equation& l1, const Equation& l2);
        static std::vector<geo::Coordinate> BezierLine(
            const geo::Bezier& B, const geo::Vector& V);
        static std::vector<geo::Coordinate> BezierCircle(
            const geo::Bezier& B, const geo::Circle& C);
        static std::vector<geo::Coordinate> BezierArc(
            const geo::Bezier& B, const geo::Arc& A);
        static std::vector<geo::Coordinate> BezierEllipse(
            const geo::Bezier& B, const geo::Ellipse& E);
        static std::vector<geo::Coordinate> BezierBezier(
            const geo::Bezier& B1, const geo::Bezier& B2);
        };
    }
}
