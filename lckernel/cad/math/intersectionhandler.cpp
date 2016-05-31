#include "cad/math/intersectionhandler.h"
using namespace lc;
using namespace maths;

std::vector<geo::Coordinate> Intersection::LineLine(const Equation& l1,
                                          const Equation& l2) {
    std::vector<lc::geo::Coordinate> ret;
    const auto &m1 = l1.Matrix();
    const auto &m2 = l2.Matrix();
//    std::vector<std::vector<double>> ce = {
//        // D, E, F
//        {m1(2,0) + m1(0,2), m1(2,1) + m1(1,2), -m1(2,2)},
//        {m2(2,0) + m2(0,2), m2(2,1) + m2(1,2), -m2(2,2)}
//    };

//    std::vector<double> sn(2, 0.);
//    if (Math::linearSolver(ce, sn)) {
//        ret.emplace_back(sn[0], sn[1]);
//    }
//    return ret;
    Eigen::Matrix2d M;
    Eigen::Vector2d V;

    M << m1(2,0) + m1(0,2), m1(2,1) + m1(1,2),
         m2(2,0) + m2(0,2), m2(2,1) + m2(1,2);
    V << -m1(2,2), -m2(2,2);

    Eigen::Vector2d R = M.colPivHouseholderQr().solve(V);
    ret.emplace_back(R[0], R[1]);
    return ret;
}

std::vector<lc::geo::Coordinate> Intersection::LineQuad(const Equation& l1,
                                          const Equation& q1) {
    auto &&tcoords = QuadQuad(l1.flipXY(), q1.flipXY());
    std::transform(tcoords.begin(), tcoords.end(), tcoords.begin(), [](const lc::geo::Coordinate &c)  { return std::move(c.flipXY()); });
    return tcoords;
}

std::vector<lc::geo::Coordinate> Intersection::QuadQuad(const Equation& l1,
                                           const Equation& l2) {
    const auto &m1 = l1.Matrix();
    const auto &m2 = l2.Matrix();

    if (std::abs(m1(0, 0)) < LCTOLERANCE && std::abs(m1(0, 1)) < LCTOLERANCE
        &&
        std::abs(m2(0, 0)) < LCTOLERANCE && std::abs(m2(0, 1)) < LCTOLERANCE
       ) {
        if (std::abs(m1(1, 1)) < LCTOLERANCE && std::abs(m2(1, 1)) < LCTOLERANCE) {
            return LineLine(l1, l2);
        }
        return LineQuad(l1,l2);
    }

    std::vector<std::vector<double> >  ce(0);
    ce.push_back(l1.Coefficients());
    ce.push_back(l2.Coefficients());
    return Math::simultaneousQuadraticSolverFull(ce);
}
