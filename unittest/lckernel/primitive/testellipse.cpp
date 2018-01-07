//
// Created by modest on 1/7/18.
//

#include <gtest/gtest.h>
#include <cad/primitive/ellipse.h>

using namespace lc;
using namespace entity;

TEST(lc__entity__EllipseTest, boundingBox) {
    Ellipse el(geo::Coordinate(0,0), geo::Coordinate(20,20), 10, 0, 0, false,nullptr);
    auto res = el.boundingBox();
    ASSERT_NEAR(res.minP().x(), -21.213203435,0.00000001);
    ASSERT_NEAR(res.minP().y(), -21.213203435,0.00000001);
    ASSERT_NEAR(res.maxP().x(), 21.213203435,0.00000001);
    ASSERT_NEAR(res.maxP().x(), 21.213203435,0.00000001);
}