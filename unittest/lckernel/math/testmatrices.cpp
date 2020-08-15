#include <gtest/gtest.h>
#include "cad/math/equation.h"
#include "cad/math/intersectionhandler.h"
#include "cad/geometry/geovector.h"
#include "cad/math/lcmath.h"
#include <iostream>
#include <unsupported/Eigen/Polynomials>

using namespace std;

TEST(EIGEN, QUARTICFULL) {
    std::vector<double> z = {1080, -126, -123, 6, 3 };
    std::vector<double>res = {-4,3,-6,5};
    auto v = lc::maths::Math::quarticSolverFull(z);
    for(unsigned long i = 0; i < v.size(); i++) {
        ASSERT_NEAR(res[i], v[i], 1e-6);
    }

    std::vector<double> z2 = {2,-41,-42, 360};
    auto v2 = lc::maths::Math::quarticSolver(z2);
    for(unsigned long i = 0; i < v2.size(); i++) {
        ASSERT_NEAR(res[i], v2[i], 1e-6);
    }
}

TEST(Matrix, Move) {
    auto x = lc::maths::Equation(1,2,3,4,5,6).move(lc::geo::Coordinate(5,5)).Coefficients();

    std::vector<int> y{ 1, 2, 3, -16, -35, 111 };
    ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

    for (unsigned long i = 0; i < x.size(); ++i) {
        EXPECT_EQ(y[i], x[i]) << "Vectors x and y differ at index " << i;
    }
}

TEST(Matrix, Rotate) {
    auto x = lc::maths::Equation(1,2,3,4,5,6).rotate(20).Coefficients();
    std::vector<double> y{1.92182, -2.8241, 2.07818, -2.9324, 5.69219, 6};

    ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

    for (unsigned long i = 0; i < x.size(); ++i) {
        ASSERT_NEAR(x[i], y[i], 1e-5) << "Vectors x and y differ at index " << i;
    }
}

TEST(QM, QUADQUAD) {
    auto _r1 = 5;
    auto _r2 = 5;
    auto _c1 = lc::geo::Coordinate(0.,0.);
    auto _c2 = lc::geo::Coordinate(6.,0.);

    lc::maths::Equation ret(1.,0.,1.,0,.0,-_r1* _r1);
    auto qm1 = ret.move(_c1);
    lc::maths::Equation ret2(1.,0.,1.,0,.0,-_r2 * _r2);
    auto qm2 = ret2.move(_c2);

    std::vector<lc::geo::Coordinate> x = lc::maths::Intersection::QuadQuad(qm1, qm2);
    ASSERT_EQ(x.size(), 2);
    EXPECT_DOUBLE_EQ(x[0].x(), 3) << "X differs";
    EXPECT_DOUBLE_EQ(x[0].y(), 4) << "Y differs";
    EXPECT_DOUBLE_EQ(x[1].x(), 3) << "X differs";
    EXPECT_DOUBLE_EQ(x[1].y(),-4) << "Y differs";
}

TEST(QM, LineLine) {
    lc::geo::Vector _l1 = lc::geo::Vector(lc::geo::Coordinate(0,0), lc::geo::Coordinate(5,5));
    lc::geo::Vector _l2 = lc::geo::Vector(lc::geo::Coordinate(5,0), lc::geo::Coordinate(0,5));
    auto eq1 = _l1.equation();
    auto eq2 = _l2.equation();

    std::vector<lc::geo::Coordinate> x = lc::maths::Intersection::LineLine(eq1, eq2);
    EXPECT_DOUBLE_EQ(x[0].x(), 2.5) << "X differs";
    EXPECT_DOUBLE_EQ(x[0].y(), 2.5) << "Y differs";
}

TEST(QM, LineQuad) {
    lc::geo::Vector _l1 = lc::geo::Vector(lc::geo::Coordinate(0,0), lc::geo::Coordinate(0,10));
    auto circle = lc::geo::Circle(lc::geo::Coordinate(0,0), 10);

    auto lqm = _l1.equation();
    auto cqm = circle.equation();

    std::vector<lc::geo::Coordinate> x = lc::maths::Intersection::LineQuad(cqm, lqm);
    EXPECT_DOUBLE_EQ(x[0].x(), 0) << "X differs";
    EXPECT_DOUBLE_EQ(x[0].y(), 10) << "Y differs";
    EXPECT_DOUBLE_EQ(x[1].x(), 0) << "X differs";
    EXPECT_DOUBLE_EQ(x[1].y(),-10) << "Y differs";
}

TEST(Maths, LinearSolver) {
    Eigen::Matrix2d M;
    Eigen::Vector2d V;

    M << 1, -1, 3, 1;
    V << -1, 9;
    Eigen::Vector2d res = M.colPivHouseholderQr().solve(V);

    EXPECT_DOUBLE_EQ(res[0], 2) << "X differs";
    EXPECT_DOUBLE_EQ(res[1], 3) << "Y differs";
}
