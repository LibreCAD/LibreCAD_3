#include <cad/geometry/geocoordinate.h>
#include <cad/base/visitor.h>
#include <cad/math/intersect.h>
#include <gtest/gtest.h>

//
// Created by R. van Twisk on 5/6/15.
//
TEST(IntersectTest, LineLine1) {
    lc::entity::CADEntity_CSPtr i1=std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0),  lc::geo::Coordinate(10,10), nullptr);
    lc::entity::CADEntity_CSPtr i2=std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,10), lc::geo::Coordinate(10,0), nullptr);
    lc::maths::Intersect intersect(lc::maths::Intersect::OnEntity, LCTOLERANCE);
    visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, *i1.get(), *i2.get());
    lc::geo::Coordinate c = intersect.result().at(0);
    if (intersect.result().size()==1) {
        c = intersect.result().at(0);
        EXPECT_DOUBLE_EQ(5., c.x());
        EXPECT_DOUBLE_EQ(5., c.y());
        EXPECT_DOUBLE_EQ(0., c.z());
    }
    if (intersect.result().size()!=1) {
        FAIL() << "Expected only one result but got " << intersect.result().size() << " result(s).";
    }
}

TEST(IntersectTest, LineLine2) {
    lc::entity::CADEntity_CSPtr i1=std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0),  lc::geo::Coordinate(10,10), nullptr);
    lc::entity::CADEntity_CSPtr i2=std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,50), lc::geo::Coordinate(50,0), nullptr);
    lc::maths::Intersect intersect(lc::maths::Intersect::OnEntity, LCTOLERANCE);
    visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, *i1.get(), *i2.get());
    EXPECT_EQ(0, intersect.result().size());

    lc::maths::Intersect intersect2(lc::maths::Intersect::OnPath, LCTOLERANCE);
    visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect2, *i1.get(), *i2.get());
    if (intersect2.result().size()==1) {
        lc::geo::Coordinate c = intersect2.result().at(0);
        EXPECT_DOUBLE_EQ(25., c.x());
        EXPECT_DOUBLE_EQ(25., c.y());
        EXPECT_DOUBLE_EQ(0., c.z());
    }
    if (intersect2.result().size()!=1) {
        FAIL() << "Expected only one result but got " << intersect.result().size() << " result(s).";
    }
}

