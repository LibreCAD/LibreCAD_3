#include <cad/geometry/geocoordinate.h>
#include <cmath>
#include <gtest/gtest.h>

TEST(GeoCoordinate, RotateInXYPlanePreservesElevation) {
    const lc::geo::Coordinate point(3.0, 4.0, 7.0);
    const auto aroundOrigin = point.rotate(std::acos(-1.0) / 2.0);
    EXPECT_NEAR(aroundOrigin.x(), -4.0, 1e-12);
    EXPECT_NEAR(aroundOrigin.y(), 3.0, 1e-12);
    EXPECT_DOUBLE_EQ(aroundOrigin.z(), 7.0);

    const auto aroundCenter = point.rotate(lc::geo::Coordinate(1.0, 2.0, 100.0), std::acos(-1.0) / 2.0);
    EXPECT_NEAR(aroundCenter.x(), -1.0, 1e-12);
    EXPECT_NEAR(aroundCenter.y(), 4.0, 1e-12);
    EXPECT_DOUBLE_EQ(aroundCenter.z(), 7.0);
    EXPECT_DOUBLE_EQ(point.rotate(lc::geo::Coordinate(1.0, 2.0, 1e30),
                                  std::acos(-1.0) / 2.0).z(), 7.0);

    const auto planar = lc::geo::Coordinate(3.0, 4.0).rotate(std::acos(-1.0) / 2.0);
    EXPECT_DOUBLE_EQ(planar.z(), 0.0);
}

TEST(GeoCoordinate, ScaleUsesAllCoordinateComponents) {
    const lc::geo::Coordinate point(3., 4., 7.);
    const lc::geo::Coordinate factors(2., -3., 4.);
    EXPECT_EQ(point.scale(-2.), point * -2.);
    EXPECT_EQ(point.scale(factors), point * factors);
    EXPECT_DOUBLE_EQ(point.scale(-2.).z(), -14.);
    EXPECT_DOUBLE_EQ(point.scale(factors).z(), 28.);
    EXPECT_DOUBLE_EQ(point.scale(lc::geo::Coordinate(2., 3.)).z(), 0.);
    EXPECT_DOUBLE_EQ(lc::geo::Coordinate(3., 4.).scale(-2.).z(), 0.);
}
