#include <cmath>
#include <cad/math/lcmath.h>
#include <gtest/gtest.h>

//
// Created by R. van Twisk on 5/5/15.
//

#pragma once

TEST(MathTest, correctAngle) {
    EXPECT_DOUBLE_EQ(0., lc::Math::correctAngle(0.));
    EXPECT_DOUBLE_EQ(.5 * M_PI, lc::Math::correctAngle(2 * M_PI + .5 * M_PI));
    EXPECT_DOUBLE_EQ(-.5 * M_PI, lc::Math::correctAngle(2 * M_PI - .5 * M_PI));
    EXPECT_DOUBLE_EQ(-.5 * M_PI, lc::Math::correctAngle(2 * M_PI - .5 * M_PI));
    EXPECT_DOUBLE_EQ(-.5 * M_PI, lc::Math::correctAngle(1.5 * M_PI));

}

TEST(MathTest, isAngleBetween) {
    EXPECT_TRUE (lc::Math::isAngleBetween(0.5 * M_PI, 0., M_PI, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(-0.5 * M_PI, M_PI, 0., true));
    EXPECT_TRUE (lc::Math::isAngleBetween(-0.5 * M_PI, -M_PI, 0, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(M_PI, -M_PI, M_PI, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(0., -M_PI, M_PI, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(M_PI, 0.5 * M_PI, -0.5 * M_PI, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(0, 0.5 * M_PI, -0.5 * M_PI, false));

    EXPECT_TRUE (lc::Math::isAngleBetween(1., 0., M_PI, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(M_PI, 0.1, -0.1, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(0, 0.1, -0.1, false));
    EXPECT_FALSE(lc::Math::isAngleBetween(M_PI, 0.1, -0.1, false));
    EXPECT_TRUE (lc::Math::isAngleBetween(1., 0., M_PI, true));
    EXPECT_FALSE(lc::Math::isAngleBetween(1., 0., M_PI, false));

    EXPECT_TRUE (lc::Math::isAngleBetween(-0.5 * M_PI, M_PI, 0, true));
    EXPECT_FALSE(lc::Math::isAngleBetween(-0.5 * M_PI, M_PI, 0, false));
    EXPECT_TRUE (lc::Math::isAngleBetween(-0.5 * M_PI, -M_PI, 0, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(-0.5 * M_PI, M_PI, 0, true));

    EXPECT_TRUE (lc::Math::isAngleBetween(0.5 * M_PI, M_PI, 0, false));
    EXPECT_TRUE (lc::Math::isAngleBetween(0.5 * M_PI, 0, M_PI, true));
    EXPECT_FALSE (lc::Math::isAngleBetween(0.5 * M_PI, 0,- M_PI, false));
}