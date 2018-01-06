//
// Created by modest on 06.01.18.
//

#include <gtest/gtest.h>
#include <cad/geometry/geoellipse.h>

using namespace lc;
using namespace geo;

TEST(lc__geo__EllipseTest, nearestPointOnPath) {

    ASSERT_NEAR(Ellipse(Coordinate(0., 0.),Coordinate(20.,0.), 10, 0, 0)
                .nearestPointOnPath(Coordinate(10.,13.))
                .distanceTo(Coordinate(8.97896136, 8.9355785)), 0, 1.0e-7);

    ASSERT_NEAR(Ellipse(Coordinate(-3., 7.).rotate(Coordinate(0.,0.),0.34), Coordinate(20.,0.).rotate(Coordinate(0.,0.),0.34), 10, 0, 0)
                .nearestPointOnPath(Coordinate(7.,20.).rotate(Coordinate(0.,0.), 0.34))
                .distanceTo(Coordinate(5.97896136, 15.9355785).rotate(Coordinate(0.,-0.), 0.34)), 0, 1.0e-7);
}
