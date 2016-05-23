#include "cad/math/equation.h"
#include "cad/math/quadratic_math.h"
#include "cad/math/intersectionhandler.h"
#include "cad/geometry/geovector.h"
#include "cad/math/lcmath.h"
#include <iostream>

using namespace std;

TEST(Matrix, Move) {
    lc::Quadratic qs(1,2,3,4,5,6);
    lc::maths::Equation qm(1,2,3,4,5,6);

    qs.move(lc::geo::Coordinate(5,5));
    auto ret = qm.moved(lc::geo::Coordinate(5,5));
    auto x = qs.getCoefficients();
    auto y = ret.Coefficients();
    ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

    for (int i = 0; i < x.size(); ++i) {
        EXPECT_EQ(x[i], y[i]) << "Vectors x and y differ at index " << i;
    }
}

TEST(Matrix, Rotate) {
    lc::Quadratic qs(1,2,3,4,5,6);
    lc::maths::Equation qm(1,2,3,4,5,6);

    qs.rotate(20);
    auto ret = qm.rotated(20);
    auto x = qs.getCoefficients();
    auto y = ret.Coefficients();
    ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

    for (int i = 0; i < x.size(); ++i) {
        EXPECT_EQ(x[i], y[i]) << "Vectors x and y differ at index " << i;
    }
}

TEST(QM, QUADQUAD) {
    auto _r1 = 5;
    auto _r2 = 5;
    auto _c1 = lc::geo::Coordinate(0.,0.);
    auto _c2 = lc::geo::Coordinate(2.,2.);

    lc::maths::Equation ret(1.,0.,1.,0,.0,-_r1* _r1);
    auto qm1 = ret.moved(_c1);
    lc::maths::Equation ret2(1.,0.,1.,0,.0,-_r2 * _r2);
    auto qm2 = ret2.moved(_c2);

    lc::Quadratic qr(1.,0.,1.,0,.0,-_r1* _r1);
    qr.move(_c1);
    lc::Quadratic qr2(1.,0.,1.,0,.0,-_r2 * _r2);
    qr2.move(_c2);

    std::vector<lc::geo::Coordinate> x = lc::Quadratic::getIntersection(qr, qr2);
    std::vector<lc::geo::Coordinate> y = lc::maths::Intersection::QuadQuad(qm1, qm2);

    ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

    for (int i = 0; i < x.size(); ++i) {
        EXPECT_DOUBLE_EQ(x[i].x(), y[i].x()) << "X differs at index " << i;
        EXPECT_DOUBLE_EQ(x[i].y(), y[i].y()) << "Y differs at index " << i;
    }
}

TEST(QM, LineLine) {
    lc::geo::Vector _l1 = lc::geo::Vector(lc::geo::Coordinate(-5,-5), lc::geo::Coordinate(5,5));
    lc::geo::Vector _l2 = lc::geo::Vector(lc::geo::Coordinate(5,0), lc::geo::Coordinate(0,5));

    auto&& dvp1 = _l1.end() - _l1.start();
    lc::geo::Coordinate normal1(-dvp1.y(), dvp1.x());
    lc::maths::Equation ret1(0,0,0,normal1.x(),normal1.y(),- normal1.dot(_l1.end()));

    auto&& dvp2 = _l2.end() - _l2.start();
    lc::geo::Coordinate normal2(-dvp2.y(), dvp2.x());
    lc::maths::Equation ret2(0,0,0,normal2.x(),normal2.y(),- normal2.dot(_l2.end()));

    auto ql1 = lc::Quadratic(normal1.x(), normal1.y(), -normal1.dot(_l1.end()));
    auto ql2 = lc::Quadratic(normal2.x(), normal2.y(), -normal2.dot(_l2.end()));

    std::vector<lc::geo::Coordinate> x = lc::Quadratic::getIntersection(ql1, ql2);
    std::vector<lc::geo::Coordinate> y = lc::maths::Intersection::LineLine(ret1, ret2);

    ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

    for (int i = 0; i < x.size(); ++i) {
        EXPECT_DOUBLE_EQ(x[i].x(), y[i].x()) << "X differs at index " << i;
        EXPECT_DOUBLE_EQ(x[i].y(), y[i].y()) << "Y differs at index " << i;
    }
}

TEST(QM, LineQuad) {
    lc::geo::Vector _l1 = lc::geo::Vector(lc::geo::Coordinate(0,0), lc::geo::Coordinate(15,15));

    auto&& dvp1 = _l1.end() - _l1.start();
    lc::geo::Coordinate normal1(-dvp1.y(), dvp1.x());

    auto lqm = _l1.equation();
    auto lq = lc::Quadratic(normal1.x(), normal1.y(), -normal1.dot(_l1.end()));

    auto _r1 = 200;
    auto _c1 = lc::geo::Coordinate(0.,0.);

    auto circle = lc::geo::Circle(lc::geo::Coordinate(0,0), 200);
    auto cqm = circle.equation();
    lc::Quadratic cq(1.,0.,1.,0,.0,-_r1* _r1);
    cq.move(_c1);

    std::vector<lc::geo::Coordinate> x = lc::Quadratic::getIntersection(lq, cq);
    std::vector<lc::geo::Coordinate> y = lc::maths::Intersection::LineQuad(cqm, lqm);

    ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

    for (int i = 0; i < x.size(); ++i) {
        EXPECT_DOUBLE_EQ(x[i].x(), y[i].x()) << "X differs at index " << i;
        EXPECT_DOUBLE_EQ(x[i].y(), y[i].y()) << "Y differs at index " << i;
    }
}

TEST(Maths, LinearSolver) {
    std::vector<std::vector<double>> a {{1,-1, -1}, {3,1, 9}};
    std::vector<double> res1;
    Eigen::Matrix2d M;
    Eigen::Vector2d V;

    M << 1, -1, 3, 1;
    V << -1, 9;
    lc::Math::linearSolver(a, res1);
    Eigen::Vector2d res2 = M.colPivHouseholderQr().solve(V);

    EXPECT_DOUBLE_EQ(res1[0], res2[0]) << "X differs";
    EXPECT_DOUBLE_EQ(res1[1], res2[1]) << "Y differs";
}
