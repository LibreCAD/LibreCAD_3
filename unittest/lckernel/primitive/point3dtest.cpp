#include <cad/primitive/point.h>
#include <cmath>
#include <gtest/gtest.h>

namespace {
using lc::geo::Coordinate;
using lc::entity::Point;
using lc::entity::Point_CSPtr;

Point_CSPtr asPoint(const lc::entity::CADEntity_CSPtr& value) {
    return std::dynamic_pointer_cast<const Point>(value);
}

void expectPosition(const Point_CSPtr& point, double x, double y, double z) {
    ASSERT_NE(point, nullptr);
    EXPECT_NEAR(point->x(), x, 1e-12);
    EXPECT_NEAR(point->y(), y, 1e-12);
    EXPECT_DOUBLE_EQ(point->z(), z);
}
} // namespace

TEST(Point3D, EditingAndCopyPreserveCoordinatesAndIdentity) {
    const auto point = std::make_shared<Point>(Coordinate(3, 4, 7), nullptr);
    const Point_CSPtr source = point;
    const Point copied(source, true);
    EXPECT_DOUBLE_EQ(copied.z(), 7.0);
    EXPECT_EQ(copied.id(), point->id());

    const auto moved = asPoint(point->move(Coordinate(2, -1, 5)));
    expectPosition(moved, 5, 3, 12);
    ASSERT_NE(moved, nullptr);
    EXPECT_EQ(moved->id(), point->id());
    const auto duplicate = asPoint(point->copy(Coordinate(2, -1, 5)));
    expectPosition(duplicate, 5, 3, 12);
    ASSERT_NE(duplicate, nullptr);
    EXPECT_NE(duplicate->id(), point->id());

    const auto rotated = asPoint(point->rotate(Coordinate(1, 2, 100), std::acos(-1.0) / 2.0));
    expectPosition(rotated, -1, 4, 7);
    ASSERT_NE(rotated, nullptr);
    EXPECT_EQ(rotated->id(), point->id());
    expectPosition(asPoint(point->scale(Coordinate(1, 2, 3), Coordinate(2, 3, 4))), 5, 8, 19);
    expectPosition(asPoint(point->mirror(Coordinate(0, 0, 42), Coordinate(1, 0, 42))), 3, -4, 7);
    const Coordinate axis1(0, 0, 0), axis2(1, 2, 3);
    const auto expected = Coordinate(3, 4, 7).mirror(axis1, axis2);
    expectPosition(asPoint(point->mirror(axis1, axis2)), expected.x(), expected.y(), expected.z());
    const auto unchanged = asPoint(point->mirror(axis1, axis1));
    expectPosition(unchanged, 3, 4, 7);
    ASSERT_NE(unchanged, nullptr);
    EXPECT_EQ(unchanged->id(), point->id());
    const auto modified = asPoint(point->modify(nullptr, nullptr, nullptr));
    expectPosition(modified, 3, 4, 7);
    ASSERT_NE(modified, nullptr);
    EXPECT_EQ(modified->id(), point->id());
    const auto planar = std::make_shared<Point>(3.0, 4.0, nullptr);
    expectPosition(asPoint(planar->rotate(Coordinate(1, 2), std::acos(-1.0) / 2.0)), -1, 4, 0);
    expectPosition(asPoint(planar->mirror(Coordinate(0, 0), Coordinate(1, 0))), 3, -4, 0);
    const Point stackPoint(Coordinate(3, 4, 7), nullptr);
    expectPosition(asPoint(stackPoint.mirror(axis1, axis1)), 3, 4, 7);
}

TEST(Point3D, BoundsSnapAndDragRetainElevation) {
    const auto point = std::make_shared<Point>(Coordinate(3, 4, 7), nullptr);
    EXPECT_DOUBLE_EQ(point->boundingBox().minP().z(), 7);
    EXPECT_DOUBLE_EQ(point->boundingBox().maxP().z(), 7);
    EXPECT_DOUBLE_EQ(point->nearestPointOnPath(Coordinate()).z(), 7);
    EXPECT_DOUBLE_EQ(point->nearestPointOnEntity(Coordinate()).z(), 7);
    EXPECT_DOUBLE_EQ(point->dragPoints().at(0).z(), 7);
    const auto snap = point->snapPoints(Coordinate(3, 4, 7), lc::SimpleSnapConstrain(), 10, 1);
    ASSERT_EQ(snap.size(), 1u);
    EXPECT_DOUBLE_EQ(snap.front().coordinate().z(), 7);
    const auto dragged = asPoint(point->setDragPoints({{0, Coordinate(8, 9, 10)}}));
    expectPosition(dragged, 8, 9, 10);
    ASSERT_NE(dragged, nullptr);
    EXPECT_EQ(dragged->id(), point->id());
}
