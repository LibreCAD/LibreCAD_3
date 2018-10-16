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
    auto&& tcoords = QuadQuad(l1.flipXY(), q1.flipXY());
    std::transform(tcoords.begin(), tcoords.end(), tcoords.begin(), [](const lc::geo::Coordinate &c) {
        return c.flipXY();
    });

    return tcoords;
}

std::vector<lc::geo::Coordinate> Intersection::QuadQuad(const Equation& l1, const Equation& l2) {
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


std::vector<geo::Coordinate> Intersection::bezierLine(const geo::BB_CSPtr& B, const geo::Vector& V) {
    std::vector<geo::Coordinate> ret;
    std::vector<double> roots;

    auto pts = B->getCP();

    if(pts.size()==3) {
        auto ml = geo::Vector(V.start() - V.start(), V.end() - V.start());
        auto rotate_angle = -ml.Angle1();
        auto cps = B->move(-V.start())->rotate(ml.start(), rotate_angle)->getCP();

        auto t2 = cps[0].y() - 2*cps[1].y() + cps[2].y();
        auto t1 = 2*(cps[1].y() - cps[0].y())/t2;
        auto coeff = cps[0].y()/t2;

        roots = lc::maths::Math::quadraticSolver({t1, coeff});
    }
    else {
        auto ml = geo::Vector(V.start() - V.start(), V.end() - V.start());
        auto rotate_angle = -ml.Angle1();
        auto cps = B->move(-V.start())->rotate(ml.start(), rotate_angle)->getCP();

        auto t3 = -cps[0].y() + 3*cps[1].y() - 3*cps[2].y() + cps[3].y();
        auto t2 = (3*cps[0].y() - 6*cps[1].y() + 3*cps[2].y())/t3;
        auto t1 = (-3*cps[0].y() +3*cps[1].y())/t3;
        auto coeff = cps[0].y()/t3;

        roots = lc::maths::Math::cubicSolver({t2, t1, coeff});
    }
    for(const auto& root : roots) {
        if(root > 0 && root < 1) {
            ret.push_back(B->DirectValueAt(root));
        }
    }

    return ret;
}


std::vector<geo::Coordinate> Intersection::bezierCircle(const geo::BB_CSPtr& B, const geo::Circle& C) {
    std::vector<geo::Coordinate> ret;

    // ((x0 (1-t)^2+ x1 (2 t - 2 t^2) + x2 t^2) - xC)^2 + ((y0 (1-t)^2+ y1 (2 t - 2 t^2) + y2 t^2) - yC)^2 = r2

    // (((a - 2b + c)t^2 + 2t(b-a) + a) - d)^2 + (((e - 2f + g)t^2 + 2t(f-e) + e) - h)^2 - r^2

    // Solving this for T will get the required intersection
    std::vector<double>roots;

    auto points = B->getCP();

    if(points.size()== 3) {

        auto r = C.radius();
        auto d = C.center().x();
        auto h = C.center().y();

        auto a = points.at(0).x();
        auto b = points.at(1).x();
        auto c = points.at(2).x();

        auto e = points.at(0).y();
        auto f = points.at(1).y();
        auto g = points.at(2).y();

        auto t4 = (g*g + (2*e - 4*f)*g + 4* f*f - 4* e * f + e*e + c*c + (2*a-4*b)*c + 4*b*b - 4*a*b + a*a);
        auto t3 = ((4*f - 4*e)*g - 8*f*f + 12*e*f - 4*e*e + (4*b - 4*a)*c - 8*b*b + 12*a*b -4*a*a)/t4;
        auto t2 = ((-2*g + 4*f -2*e)*h + 2*e*g + 4*f*f - 12*e*f + 6*e*e + (-2*c + 4*b - 2*a)*d + 2*a*c + 4*b*b -12*a*b + 6*a*a)/t4;
        auto t1 = ((4*e - 4*f)*h + 4*e*f - 4*e*e + (4*a - 4*b)*d + 4*a*b -4*a*a)/t4;
        auto coeff = (-r*r + h*h -2 *e*h + e*e + d*d - 2*a*d + a*a)/t4;

        roots = lc::maths::Math::quarticSolver({t3, t2, t1, coeff});

        for(const auto &root : roots) {
            if(root > 0 && root < 1) {
                ret.push_back(B->DirectValueAt(root));
            }
        }

    } else {
        ret = bezCircleIntersect(B, C.center(), C.radius(), C.radius());
    }
    return ret;
}


std::vector<geo::Coordinate> Intersection::bezierArc(const geo::BB_CSPtr& B, const geo::Arc& A) {

    // BezierCircle Intersection

    // Check intersection points are on Arc.

    std::vector<geo::Coordinate> ret;
    const auto &points = bezierCircle(B, geo::Circle(A.center(), A.radius()));

    for(const auto & pt : points) {
        if(A.isAngleBetween(pt.angle())) {
            ret.push_back(pt);
        }
    }
    return ret;
}


std::vector<geo::Coordinate> Intersection::bezierEllipse(const geo::BB_CSPtr& B, const geo::Ellipse& E) {
    std::vector<double> roots;
    std::vector<geo::Coordinate> arc_ret, ret;


    auto C = geo::Ellipse(E.center() - E.center(), E.majorP(), E.minorRadius(), E.startAngle(), E.endAngle())
            .georotate(geo::Coordinate(0,0), -E.getAngle())
            .geoscale(geo::Coordinate(0,0), geo::Coordinate(1/E.ratio(),1));

    auto bez = B->move(-E.center())
            ->rotate(geo::Coordinate(0,0), E.getAngle())
            ->scale(geo::Coordinate(0,0), geo::Coordinate(1/E.ratio(),1));


    auto points = B->getCP();

    if(points.size()== 3) {
        auto r = C.minorRadius();
        auto d = C.center().x();
        auto h = C.center().y();

        auto a = points.at(0).x();
        auto b = points.at(1).x();
        auto c = points.at(2).x();

        auto e = points.at(0).y();
        auto f = points.at(1).y();
        auto g = points.at(2).y();

        auto t4 = (g*g + (2*e - 4*f)*g + 4* f*f - 4* e * f + e*e + c*c + (2*a-4*b)*c + 4*b*b - 4*a*b + a*a);
        auto t3 = ((4*f - 4*e)*g - 8*f*f + 12*e*f - 4*e*e + (4*b - 4*a)*c - 8*b*b + 12*a*b -4*a*a)/t4;
        auto t2 = ((-2*g + 4*f -2*e)*h + 2*e*g + 4*f*f - 12*e*f + 6*e*e + (-2*c + 4*b - 2*a)*d + 2*a*c + 4*b*b -12*a*b + 6*a*a)/t4;
        auto t1 = ((4*e - 4*f)*h + 4*e*f - 4*e*e + (4*a - 4*b)*d + 4*a*b -4*a*a)/t4;
        auto coeff = (-r*r + h*h -2 *e*h + e*e + d*d - 2*a*d + a*a)/t4;

        roots = lc::maths::Math::quarticSolver({t3, t2, t1, coeff});

    } else {
        // TODO CUBIC BEZIER/ELLIPSE INTERSECTION
    }

    for(const auto &root : roots) {
        if(root > 0 && root < 1) {
            ret.push_back(B->DirectValueAt(root));
        }
    }
    if(E.isArc()) {
        for(const auto &r: ret) {
            if(E.isAngleBetween(r.angle())) {
                arc_ret.push_back(r);
            }
        }
        return arc_ret;
    }
    return ret;
}

std::vector<geo::Coordinate> Intersection::bezierBezier(const geo::BB_CSPtr& B1, const geo::BB_CSPtr& B2) {
    std::vector<geo::Coordinate> ret;
    bezBez(B1,B2, ret);
    return ret;
}

void Intersection::bezBez(const geo::BB_CSPtr& B1,const geo::BB_CSPtr& B2, std::vector<geo::Coordinate>&ret) {
    auto bb1 = B1->boundingBox();
    auto bb2 = B2->boundingBox();

    if(!bb1.overlaps(bb2)) {
        return;
    }

    if(bb1.height() + bb2.height() <= BBHEURISTIC2 && bb1.width() + bb2.width() <= BBHEURISTIC2) {
        ret.push_back(B1->getCP().at(1));
        return;
    }

    auto b1split = B1->splitHalf();
    auto b2split = B2->splitHalf();
    bezBez(b1split[0], b2split[0], ret);
    bezBez(b1split[1], b2split[0], ret);
    bezBez(b1split[0], b2split[1], ret);
    bezBez(b1split[1], b2split[1], ret);
}

std::vector<geo::Coordinate> Intersection::bezCircleIntersect(const lc::geo::BB_CSPtr& bez,
                                                              const geo::Coordinate &ec,
                                                              double rx, double ry) {
    std::vector<geo::Coordinate> results;
    auto coords = bez->getCP();
    auto p1 = coords.at(0);
    auto p2 = coords.at(1);
    auto p3 = coords.at(2);
    auto p4 = coords.at(3);

    auto a = p1 * -1;
    auto b = p2 * 3;
    auto c = p3 * -3;
    auto d = a + b + c + p4;
    auto c3 = geo::Coordinate(d.x(), d.y());

    a = p1 * 3;
    b = p2 * -6;
    c = p3 * 3;
    d = a + b + c;
    auto c2 = geo::Coordinate(d.x(), d.y());

    a = p1 * -3;
    b = p2 * 3;
    c = a + b;
    auto c1 = geo::Coordinate(c.x(), c.y());

    auto c0 = geo::Coordinate(p1.x(), p1.y());

    auto rxrx  = rx*rx;
    auto ryry  = ry*ry;
    auto roots = lc::maths::Math::sexticSolver({
            c3.x()*c3.x()*ryry + c3.y()*c3.y()*rxrx,

            2*(c3.x()*c2.x()*ryry + c3.y()*c2.y()*rxrx),

            2*(c3.x()*c1.x()*ryry + c3.y()*c1.y()*rxrx) + c2.x()*c2.x()*ryry + c2.y()*c2.y()*rxrx,

            2*c3.x()*ryry*(c0.x() - ec.x()) + 2*c3.y()*rxrx*(c0.y() - ec.y()) +
                2*(c2.x()*c1.x()*ryry + c2.y()*c1.y()*rxrx),

            2*c2.x()*ryry*(c0.x() - ec.x()) + 2*c2.y()*rxrx*(c0.y() - ec.y()) +
                c1.x()*c1.x()*ryry + c1.y()*c1.y()*rxrx,

            2*c1.x()*ryry*(c0.x() - ec.x()) + 2*c1.y()*rxrx*(c0.y() - ec.y()),

            c0.x()*c0.x()*ryry - 2*c0.y()*ec.y()*rxrx - 2*c0.x()*ec.x()*ryry +
                c0.y()*c0.y()*rxrx + ec.x()*ec.x()*ryry + ec.y()*ec.y()*rxrx - rxrx*ryry
        });

    for(const auto& t : roots) {
        if(t > 0.000000000 && t < 1.000000000) {
            results.push_back(c3*t*t*t + c2*t*t + c1*t + c0);
        }
    }

    return results;
}

std::vector<geo::Coordinate> Intersection::splineLine(const geo::Spline& B, const geo::Vector& V) {
    std::vector<geo::Coordinate> ret;
    auto beziers = B.beziers();
    for(const auto& bezier : beziers) {
        auto vecret = bezierLine(bezier, V);
        ret.insert(ret.end(), vecret.begin(), vecret.end());
    }
    return ret;
}

std::vector<geo::Coordinate> Intersection::splineCircle(const geo::Spline& B, const geo::Circle& C) {
    std::vector<geo::Coordinate> ret;
    auto beziers = B.beziers();
    for(const auto & bezier : beziers) {
        auto vecret = bezierCircle(bezier, C);
        ret.insert(ret.end(), vecret.begin(), vecret.end());
    }
    return ret;
}

std::vector<geo::Coordinate> Intersection::splineArc(const geo::Spline& B, const geo::Arc& A) {
    std::vector<geo::Coordinate> ret;
    auto beziers = B.beziers();
    for(const auto & bezier : beziers) {
        auto vecret = bezierArc(bezier, A);
        ret.insert(ret.end(), vecret.begin(), vecret.end());
    }
    return ret;
}

std::vector<geo::Coordinate> Intersection::splineEllipse(const geo::Spline& B, const geo::Ellipse& E) {
    std::vector<geo::Coordinate> ret;
    auto beziers = B.beziers();
    for(const auto & bezier : beziers) {
        auto vecret = bezierEllipse(bezier, E);
        ret.insert(ret.end(), vecret.begin(), vecret.end());
    }
    return ret;
}

std::vector<geo::Coordinate> Intersection::splineSpline(const geo::Spline& B1, const geo::Spline& B2) {
    std::vector<geo::Coordinate> ret;
    auto beziers = B1.beziers();
    auto beziers2 = B2.beziers();
    for(const auto & bezier : beziers) {
        for(const auto & bezier2 : beziers2) {
            auto vecret = bezierBezier(bezier, bezier2);
            ret.insert(ret.end(), vecret.begin(), vecret.end());
        }
    }
    return ret;
}

std::vector<geo::Coordinate> Intersection::splineBezier(const geo::Spline& B1, const geo::BB_CSPtr& B2) {
    std::vector<geo::Coordinate> ret;
    auto beziers = B1.beziers();
    for(const auto & bezier : beziers) {
        auto vecret = bezierBezier(bezier, B2);
        ret.insert(ret.end(), vecret.begin(), vecret.end());
    }
    return ret;
}
