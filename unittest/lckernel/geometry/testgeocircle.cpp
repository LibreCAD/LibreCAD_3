#include <cmath>
#include <cad/geometry/geocircle.h>
#include <gtest/gtest.h>

//
// Created by R. van Twisk on 5/5/15.
//

using namespace lc;
using namespace geo;


TEST(lc__geo__CircleTest, nearestPointOnpath) {
    ASSERT_NEAR(0, Circle(Coordinate(0., 0.),10.).nearestPointOnPath(Coordinate(0.,10.)).distanceTo(Coordinate(0.,10.)), LCTOLERANCE);
    ASSERT_NEAR(0.82842712474619018, Circle(Coordinate(2, 2),2).nearestPointOnPath(Coordinate(0.,0.)).distanceTo(Coordinate(0.,0.)), LCTOLERANCE);
    ASSERT_NEAR(2, Circle(Coordinate(2, 2),2).nearestPointOnPath(Coordinate(2.,2.)).distanceTo(Coordinate(2.,2.)), LCTOLERANCE);
}