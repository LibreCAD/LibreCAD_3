//
// Created by modest on 06.01.18.
//

#include <cmath>
#include <gtest/gtest.h>
#include <cad/geometry/geoellipse.h>

using namespace lc;
using namespace geo;

TEST(lc__geo__EllipseTest, rotationPreservesAxisAndElevation) {
    const Ellipse ellipse(Coordinate(5.0, 7.0, 3.0), Coordinate(4.0, 0.0, 0.0),
                          2.0, 0.2, 1.7, true);
    const auto rotated = ellipse.georotate(Coordinate(1.0, 2.0, 100.0), std::acos(-1.0) / 2.0);
    EXPECT_NEAR(rotated.center().x(), -4.0, 1e-12);
    EXPECT_NEAR(rotated.center().y(), 6.0, 1e-12);
    EXPECT_DOUBLE_EQ(rotated.center().z(), 3.0);
    EXPECT_NEAR(rotated.majorP().x(), 0.0, 1e-12);
    EXPECT_NEAR(rotated.majorP().y(), 4.0, 1e-12);
    EXPECT_DOUBLE_EQ(rotated.majorP().z(), 0.0);
    EXPECT_DOUBLE_EQ(rotated.minorRadius(), 2.0);
    EXPECT_DOUBLE_EQ(rotated.startAngle(), 0.2);
    EXPECT_DOUBLE_EQ(rotated.endAngle(), 1.7);
    EXPECT_TRUE(rotated.isReversed());
    const Ellipse tilted(Coordinate(5.0, 7.0, 3.0), Coordinate(4.0, 0.0, 2.0),
                         2.0, 0.2, 1.7);
    const auto rotatedTilted = tilted.georotate(Coordinate(1.0, 2.0, 100.0), std::acos(-1.0) / 2.0);
    EXPECT_DOUBLE_EQ(rotatedTilted.center().z(), 3.0);
    EXPECT_DOUBLE_EQ(rotatedTilted.majorP().z(), 2.0);
}

TEST(lc__geo__EllipseTest, getPoint) {
    Ellipse el(Coordinate(0.,0.), Coordinate(20.,0.),10, 0, 0);
    ASSERT_NEAR(el.getPoint(0).distanceTo(Coordinate(20.,0.)), 0, 0.0000001);

    double a1 = el.getEllipseAngle(Coordinate(1, 1));
    double a2 = el.getEllipseAngle(Coordinate(-1, 1));
    double a3 = el.getEllipseAngle(Coordinate(-1, -1));
    double a4 = el.getEllipseAngle(Coordinate(1, -1));
    Coordinate c1 = el.getPoint(a1);
    Coordinate c2 = el.getPoint(a2);
    Coordinate c3 = el.getPoint(a3);
    Coordinate c4 = el.getPoint(a4);

    ASSERT_NEAR(c1.distanceTo(Coordinate(8.94427191,8.94427191)), 0, 0.0000001);
    ASSERT_NEAR(c2.distanceTo(Coordinate(-8.94427191,8.94427191)), 0, 0.0000001);
    ASSERT_NEAR(c3.distanceTo(Coordinate(-8.94427191,-8.94427191)), 0, 0.0000001);
    ASSERT_NEAR(c4.distanceTo(Coordinate(8.94427191,-8.94427191)), 0, 0.0000001);

    Ellipse el2(Coordinate(0.,0.), Coordinate(20.,0).rotate(M_PI / 4),10, 0, 0);
    a1 = el2.getEllipseAngle(Coordinate(0, 1));
    c1 = el2.getPoint(a1);
    c2 = Coordinate(8.94427191,8.94427191).rotate(M_PI / 4);
    ASSERT_NEAR(c1.distanceTo(c2), 0, 0.0000001);
}


TEST(lc__geo__EllipseTest, nearestPointOnPath) {

    ASSERT_NEAR(Ellipse(Coordinate(0., 0.),Coordinate(20.,0.), 10, 0, 0)
                .nearestPointOnPath(Coordinate(10.,13.))
                .distanceTo(Coordinate(8.97896136, 8.9355785)), 0, 1.0e-7);

    ASSERT_NEAR(Ellipse(Coordinate(-3., 7.).rotate(Coordinate(0.,0.),0.34), Coordinate(20.,0.).rotate(Coordinate(0.,0.),0.34), 10, 0, 0)
                .nearestPointOnPath(Coordinate(7.,20.).rotate(Coordinate(0.,0.), 0.34))
                .distanceTo(Coordinate(5.97896136, 15.9355785).rotate(Coordinate(0.,-0.), 0.34)), 0, 1.0e-7);
}
