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
            geo::BB_CSPtr B, const geo::Vector& V);
        static std::vector<geo::Coordinate> BezierCircle(
            geo::BB_CSPtr B, const geo::Circle& C);
        static std::vector<geo::Coordinate> BezierArc(
            geo::BB_CSPtr B, const geo::Arc& A);
        static std::vector<geo::Coordinate> BezierEllipse(geo::BB_CSPtr B, const geo::Ellipse& E);
        static std::vector<geo::Coordinate> BezierBezier(
            geo::BB_CSPtr B1, geo::BB_CSPtr B2);

        private:
        static void BezBez(const geo::BB_CSPtr B1, const geo::BB_CSPtr B2, std::vector<geo::Coordinate>&ret);
        static void BezCir(const geo::Circle C, const geo::Area c_area, const geo::Area m_area, const geo::BB_CSPtr B2, std::vector<geo::Coordinate>&ret);
        };
    }
}
