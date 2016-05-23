#pragma once

#include "cad/geometry/geocoordinate.h"
#include "cad/math/equation.h"
#include "eigen3/Eigen/Dense"

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
        };
    }
}
