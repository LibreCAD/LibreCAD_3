#include <cmath>
#include <cad/geometry/geocircle.h>
#include <gtest/gtest.h>
#include "comparecoordinate.h"

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

TEST(lc__geo__CircleTest, TangentTest) {
    Circle c1(Coordinate(0,0), 10.0);

    Coordinate coord11(-5, 8.6602540378);
    Coordinate coord12(-5, -8.6602540378);

    auto points = c1.lineTangentPointsOnEntity(Coordinate(-20, 0));
    compareCoordinate(points[0], coord11);
    compareCoordinate(points[1], coord12);

    Circle c2(Coordinate(4.72, 4.27), 10.0);
    Coordinate coord21(12.659432173581992, -1.809919140999419);
    Coordinate coord22(1.150754230108806, 13.611332058872215);

    points = c2.lineTangentPointsOnEntity(Coordinate(34.113885528037471, 26.206272074493956));
    compareCoordinate(points[0], coord21);
    compareCoordinate(points[1], coord22);
}