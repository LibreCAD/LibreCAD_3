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

    std::vector<geo::Coordinate> ret;

    // ((x0 (1-t)^2+ x1 (2 t - 2 t^2) + x2 t^2) - xC)^2 + ((y0 (1-t)^2+ y1 (2 t - 2 t^2) + y2 t^2) - yC)^2 = r2

    // (((a - 2b + c)t^2 + 2t(b-a) + a) - d)^2 + (((e - 2f + g)t^2 + 2t(f-e) + e) - h)^2 - r^2

    // Solving this for T will get the required intersection

    auto r = C.radius();
    auto d = C.center().x();
    auto h = C.center().y();

    auto a = B.pointA().x();
    auto b = B.pointB().x();
    auto c = B.pointC().x();

    auto e = B.pointA().y();
    auto f = B.pointB().y();
    auto g = B.pointC().y();

    auto t4 = (g*g + (2*e - 4*f)*g + 4* f*f - 4* e * f + e*e + c*c + (2*a-4*b)*c + 4*b*b - 4*a*b + a*a);
    auto t3 = ((4*f - 4*e)*g - 8*f*f + 12*e*f - 4*e*e + (4*b - 4*a)*c - 8*b*b + 12*a*b -4*a*a)/t4;
    auto t2 = ((-2*g + 4*f -2*e)*h + 2*e*g + 4*f*f - 12*e*f + 6*e*e + (-2*c + 4*b - 2*a)*d + 2*a*c + 4*b*b -12*a*b + 6*a*a)/t4;
    auto t1 = ((4*e - 4*f)*h + 4*e*f - 4*e*e + (4*a - 4*b)*d + 4*a*b -4*a*a)/t4;
    auto coeff = (-r*r + h*h -2 *e*h + e*e + d*d - 2*a*d + a*a)/t4;

    auto roots = lc::Math::quarticSolver({t3, t2, t1, coeff});

    for(const auto &root : roots) {
        if(root > 0 && root < 1) {
            ret.push_back(B.DirectValueAt(root));
        }
    }
    return ret;
}


std::vector<geo::Coordinate> Intersection::BezierArc(
    const geo::Bezier& B, const geo::Arc& A) {

    // BezierCircle Intersection

    // Check intersection points are on Arc.

    std::vector<geo::Coordinate> ret;
    const auto &points = BezierCircle(B, geo::Circle(A.center(), A.radius()));

    for(const auto & pt : points) {
        if(A.isAngleBetween(pt.angle())) {
            ret.push_back(pt);
        }
    }
    return ret;
}


std::vector<geo::Coordinate> Intersection::BezierEllipse(
    const geo::Bezier& B, const geo::Ellipse& E) {

    // TODO

}

std::vector<geo::Coordinate> Intersection::BezierBezier(
    const geo::Bezier& B1, const geo::Bezier& B2) {

    // Bounding Box method.

}
