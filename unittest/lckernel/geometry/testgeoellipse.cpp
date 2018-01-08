//
// Created by modest on 06.01.18.
//

#include <gtest/gtest.h>
#include <cad/geometry/geoellipse.h>

using namespace lc;
using namespace geo;

TEST(lc__geo__EllipseTest, getPoint) {
    Ellipse el(Coordinate(0.,0.), Coordinate(20.,0.),10, 0, 0);
    ASSERT_NEAR(el.getPoint(0).distanceTo(Coordinate(20.,0.)), 0, 0.0000001);
    Coordinate c1 = el.getPoint(M_PI/4);
    Coordinate c2 = el.getPoint(3*M_PI/4);
    Coordinate c3 = el.getPoint(5*M_PI/4);
    Coordinate c4 = el.getPoint(7*M_PI/4);

    ASSERT_NEAR(c1.distanceTo(Coordinate(8.94427191,8.94427191)), 0, 0.0000001);
    ASSERT_NEAR(c2.distanceTo(Coordinate(-8.94427191,8.94427191)), 0, 0.0000001);
    ASSERT_NEAR(c3.distanceTo(Coordinate(-8.94427191,-8.94427191)), 0, 0.0000001);
    ASSERT_NEAR(c4.distanceTo(Coordinate(8.94427191,-8.94427191)), 0, 0.0000001);

    Ellipse el2(Coordinate(0.,0.), Coordinate(20.,0.).rotate(0.36),10, 0, 0);
    c1 = el2.getPoint(0.36+M_PI/4);
    c2 = Coordinate(8.94427191,8.94427191).rotate(0.36);
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
