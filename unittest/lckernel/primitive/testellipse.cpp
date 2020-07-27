//
// Created by modest on 1/7/18.
//

#include <gtest/gtest.h>
#include <cad/primitive/ellipse.h>

using namespace lc;
using namespace entity;

TEST(lc__entity__EllipseTest, boundingBox) {


    Ellipse el1(geo::Coordinate(20,40), geo::Coordinate(20,20), 10, 0.001, 0, false,nullptr,nullptr);
    auto res = el1.boundingBox();
    ASSERT_NEAR(res.minP().x(), 20+-21.213203435,0.00000001);
    ASSERT_NEAR(res.minP().y(), 40+-21.213203435,0.00000001);
    ASSERT_NEAR(res.maxP().x(), 20+21.213203435,0.00000001);
    ASSERT_NEAR(res.maxP().y(), 40+21.213203435,0.00000001);


    Ellipse el2(geo::Coordinate(0,0), geo::Coordinate(20,0), 10, 0, M_PI / 4, false,nullptr,nullptr);
    res = el2.boundingBox();
    ASSERT_TRUE(el2.isArc());
    ASSERT_NEAR(res.minP().x(), 14.14213562373, LCTOLERANCE);
    ASSERT_NEAR(res.maxP().y(), 7.071067811865, LCTOLERANCE);

    Ellipse el3(geo::Coordinate(0, 0), geo::Coordinate(45,27), 13, 0, 0, false, nullptr,nullptr);
    res = el3.boundingBox();
    ASSERT_NEAR(res.minP().x(), -45.4943435398002,0.0000001);
    ASSERT_NEAR(res.minP().y(), -29.2106950595459,0.0000001);
    ASSERT_NEAR(res.maxP().x(),  45.4943435320002,0.0000001);
    ASSERT_NEAR(res.maxP().y(),  29.2106950562459,0.0000001);


    Ellipse el4(geo::Coordinate(0, 0), geo::Coordinate(45,27), 13, -0.9219, 1.432, false, nullptr,nullptr);
    res = el4.boundingBox();
    ASSERT_NEAR(res.minP().x(), -0.398323390742612,0.0000001);
    ASSERT_NEAR(res.minP().y(),   7.43458917447733,0.0000001);
    ASSERT_NEAR(res.maxP().x(),   45.4943435392462,0.0000001);
    ASSERT_NEAR(res.maxP().y(),   29.2106950598357,0.0000001);

    Ellipse el5(geo::Coordinate(285, -129), geo::Coordinate(58,36), 17.204650534085253, 1.5669968920869093, -1.2295236480943998, false, nullptr,nullptr);
    res = el5.boundingBox();
    ASSERT_NEAR(res.minP().x(), 226.294625203129,0.001);
    ASSERT_NEAR(res.minP().y(), -167.854587501749,0.001);

    ASSERT_NEAR(res.maxP().x(), 312.961423084428,0.001);
    ASSERT_NEAR(res.maxP().y(), -114.245564049154,0.001);
}

TEST(lc__entity__EllipseTest, snapPoints) {
    Ellipse el(geo::Coordinate(0.,0.),geo::Coordinate(20.,10.), 10, 0, 0, false, nullptr,nullptr);
    auto flag = SimpleSnapConstrain();
    auto res = el.snapPoints(geo::Coordinate(-5.,9), flag, 5, 10);
    ASSERT_EQ(res.size(), 1);

    Ellipse el2(geo::Coordinate(0.,0.),geo::Coordinate(20.,0.), 10, 0, 0, false, nullptr,nullptr);
    auto res2 = el2.snapPoints(geo::Coordinate(20.,10.), flag, 30, 10);
    ASSERT_EQ(res2.size(), 4);
}
