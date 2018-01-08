//
// Created by modest on 1/7/18.
//

#include <gtest/gtest.h>
#include <cad/primitive/ellipse.h>

using namespace lc;
using namespace entity;

TEST(lc__entity__EllipseTest, boundingBox) {
    Ellipse el1(geo::Coordinate(0,0), geo::Coordinate(20,20), 10, 0, 0, false,nullptr);
    auto res = el1.boundingBox();
    ASSERT_NEAR(res.minP().x(), -21.213203435,0.00000001);
    ASSERT_NEAR(res.minP().y(), -21.213203435,0.00000001);
    ASSERT_NEAR(res.maxP().x(), 21.213203435,0.00000001);
    ASSERT_NEAR(res.maxP().y(), 21.213203435,0.00000001);

    Ellipse el2(geo::Coordinate(0,0), geo::Coordinate(20,0), 10, 0, M_PI/4, false,nullptr);
    res = el2.boundingBox();
    ASSERT_NEAR(res.maxP().y(), 8.944271909999,0.00000001);
    ASSERT_NEAR(res.minP().x(), 8.944271909999,0.00000001);

}