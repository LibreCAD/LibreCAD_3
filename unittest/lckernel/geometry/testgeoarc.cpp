#include <cmath>
#include <gtest/gtest.h>
#include <cad/geometry/geoarc.h>
//
// Created by R. van Twisk on 5/5/15.
//

using namespace lc;
using namespace geo;


TEST(lc__geo__ArcTest, createArcBulge) {

    lc::geo::Arc arc = lc::geo::Arc::createArcBulge(lc::geo::Coordinate(15.,5.), lc::geo::Coordinate(5.,5.), -0.5);
    ASSERT_NEAR(10.,arc.center().x(), LCTOLERANCE);
    ASSERT_NEAR(8.75,arc.center().y(), LCTOLERANCE);
    ASSERT_NEAR(6.25,arc.radius(), LCTOLERANCE);
    ASSERT_FALSE(arc.CCW());

    arc = lc::geo::Arc::createArcBulge(lc::geo::Coordinate(15.,15.), lc::geo::Coordinate(25.,5.), 0.5);
    ASSERT_NEAR(23.75,arc.center().x(), LCTOLERANCE);
    ASSERT_NEAR(13.75,arc.center().y(), LCTOLERANCE);
    ASSERT_NEAR(8.8388347648318444,arc.radius(), LCTOLERANCE);
    ASSERT_TRUE(arc.CCW());
}

TEST(lc__geo__ArcTest, isAngleBetween) {

    lc::geo::Arc arc = lc::geo::Arc::createArcBulge(lc::geo::Coordinate(15.,5.), lc::geo::Coordinate(5.,5.), 0.5);
    ASSERT_TRUE(!arc.isAngleBetween(1.));

    ASSERT_FALSE(arc.isAngleBetween(M_PI));
    ASSERT_TRUE(arc.isAngleBetween(-0.5*M_PI));
}