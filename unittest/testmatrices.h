#include "cad/math/lcquadraticmaths.h"
#include "cad/math/quadratic_math.h"
#include <iostream>

using namespace lc;
using namespace maths;
using namespace std;

TEST(Matrix, Move) {
    lc::Quadratic qs(1,2,3,4,5,6);
    lc::maths::QuadraticMaths qm(1,2,3,4,5,6);

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
    lc::maths::QuadraticMaths qm(1,2,3,4,5,6);

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
    lc::maths::QuadraticMaths qm(1,2,3,4,5,6);
    lc::maths::QuadraticMaths qm2(6,5,4,3,2,1);

    lc::Quadratic qs(1,2,3,4,5,6);
    lc::Quadratic qs2(6,5,4,3,2,1);

    std::vector<lc::geo::Coordinate> x = lc::Quadratic::getIntersection(qs, qs2);
    std::vector<lc::geo::Coordinate> y = lc::maths::QuadraticMaths::IntersectionQuadQuad(qm, qm2);

    ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

    for (int i = 0; i < x.size(); ++i) {
      EXPECT_EQ(x[i], y[i]) << "Vectors x and y differ at index " << i;
    }
}
