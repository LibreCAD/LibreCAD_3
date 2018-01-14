#include <cmath>
#include <cad/math/lcmath.h>
#include <cad/const.h>
#include <gtest/gtest.h>

//
// Created by R. van Twisk on 5/5/15.
//

TEST(MathTest, getAngleDifferenceShort) {
    ASSERT_NEAR(0.1, lc::Math::getAngleDifferenceShort(0., 0.1, true), LCARCTOLERANCE);
    ASSERT_NEAR(M_PI, lc::Math::getAngleDifferenceShort(0., M_PI, true), LCARCTOLERANCE);
    ASSERT_NEAR(0., lc::Math::getAngleDifferenceShort(0., 2.*M_PI, true), LCARCTOLERANCE);
    ASSERT_NEAR(-0.1, lc::Math::getAngleDifferenceShort(0., 0.1, false), LCARCTOLERANCE);
}

TEST(MathTest, getAngleDifference) {
    ASSERT_NEAR(0.5*M_PI, lc::Math::getAngleDifference(0.5*M_PI, 0, false), LCARCTOLERANCE);
    ASSERT_NEAR(0.1, lc::Math::getAngleDifference(0., 0.1, true), LCARCTOLERANCE);
    ASSERT_NEAR(M_PI, lc::Math::getAngleDifference(0., M_PI, true), LCARCTOLERANCE);
    ASSERT_NEAR(M_PI, lc::Math::getAngleDifference(0., -M_PI, true), LCARCTOLERANCE);
    ASSERT_NEAR(2.*M_PI, lc::Math::getAngleDifference(0., 2.*M_PI, true), LCARCTOLERANCE);
    ASSERT_NEAR(2.* M_PI - 0.1, lc::Math::getAngleDifference(0., 0.1, false), LCARCTOLERANCE);
    ASSERT_NEAR(0, lc::Math::getAngleDifference(0.0, 0.0, false), LCARCTOLERANCE);
    ASSERT_NEAR(0, lc::Math::getAngleDifference(0.0, 0.0, true), LCARCTOLERANCE);
}

TEST(MathTest, correctAngle) {
    EXPECT_DOUBLE_EQ(0., lc::Math::correctAngle(0.));
    EXPECT_DOUBLE_EQ(.5 * M_PI, lc::Math::correctAngle(2 * M_PI + .5 * M_PI));
    EXPECT_DOUBLE_EQ(-.5 * M_PI, lc::Math::correctAngle(2 * M_PI - .5 * M_PI));
    EXPECT_DOUBLE_EQ(-.5 * M_PI, lc::Math::correctAngle(2 * M_PI - .5 * M_PI));
    EXPECT_DOUBLE_EQ(-.5 * M_PI, lc::Math::correctAngle(1.5 * M_PI));
    EXPECT_DOUBLE_EQ(0., lc::Math::correctAngle(2 * M_PI));
}

TEST(MathTest, isAngleBetween) {
    EXPECT_TRUE (lc::Math::isAngleBetween(0.5 * M_PI, 0., M_PI, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(-0.5 * M_PI, M_PI, 0., true));
    EXPECT_TRUE (lc::Math::isAngleBetween(-0.5 * M_PI, -M_PI, 0, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(M_PI, -M_PI, M_PI, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(0., -M_PI, M_PI, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(M_PI, 0.5 * M_PI, -0.5 * M_PI, true));
    EXPECT_TRUE (lc::Math::isAngleBetween(0, 0.5 * M_PI, -0.5 * M_PI, false));
    EXPECT_TRUE (lc::Math::isAngleBetween( 0.0001, 0.5 * M_PI, -0.5 * M_PI, false));
    EXPECT_TRUE (lc::Math::isAngleBetween(-0.0001, 0.5 * M_PI, -0.5 * M_PI, false));

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