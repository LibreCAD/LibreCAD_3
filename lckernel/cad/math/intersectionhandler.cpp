#include "cad/math/intersectionhandler.h"
using namespace lc;
using namespace maths;

std::vector<geo::Coordinate> Intersection::LineLine(const Equation& l1,
                                          const Equation& l2) {
    std::vector<lc::geo::Coordinate> ret;
    const auto &m1 = l1.Matrix();
    const auto &m2 = l2.Matrix();
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


std::vector<geo::Coordinate> Intersection::BezierLine(
    const geo::Bezier& B, const geo::Vector& V) {

    auto rotate_angle = V.end().angle();

    auto absA = geo::Coordinate(V.start().x() * -1, V.start().y() * -1);
    auto moved_bezier = geo::Bezier(B.pointA() + absA, B.pointB() + absA, B.pointC() + absA);

    std::cout << "MOVED BEZIER:" << moved_bezier << std::endl;

    auto rotated_bezier = moved_bezier.rotate(moved_bezier.pointA(), rotate_angle);

    std::cout << "ROTATED BEZIER:" << rotated_bezier << std::endl;

//    auto tx_ = (rotated_bezier.pointA().x() - rotated_bezier.pointB().x())/(rotated_bezier.pointA().x() - (rotated_bezier.pointB().x()*2.0) + rotated_bezier.pointC().x());
//    std::cout << tx_;
    std::vector<geo::Coordinate> ret;
    return ret;
}


std::vector<geo::Coordinate> Intersection::BezierCircle(
    const geo::Bezier& B, const geo::Circle& C) {

    // ((x0 (1-t)^2+ x1 (2 t - 2 t^2) + x2 t^2) - xC)^2 + ((y0 (1-t)^2+ y1 (2 t - 2 t^2) + y2 t^2) - yC)^2 = r2

    // Solving this for T will get the required intersection

}


std::vector<geo::Coordinate> Intersection::BezierArc(
    const geo::Bezier& B, const geo::Arc& A) {

    // BezierCircle Intersection

    // Check intersection points are on Arc.
}


std::vector<geo::Coordinate> Intersection::BezierEllipse(
    const geo::Bezier& B, const geo::Ellipse& E) {

    // TODO

}

std::vector<geo::Coordinate> Intersection::BezierBezier(
    const geo::Bezier& B1, const geo::Bezier& B2) {

    // Bounding Box method.

}
