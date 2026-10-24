// A linear dimension's angle is the direction of its dimension line, and
// moving the dimension has to move that direction with its points.

#include <cmath>
#include <memory>

#include <gtest/gtest.h>

#include <cad/meta/layer.h>
#include <cad/primitive/dimlinear.h>

namespace {

lc::entity::DimLinear_CSPtr horizontal() {
    return std::make_shared<lc::entity::DimLinear>(
        lc::geo::Coordinate(100, 25), lc::geo::Coordinate(50, 25),
        lc::TextConst::AttachmentPoint::Middle_center, 0.0, 1.0,
        lc::TextConst::LineSpacingStyle::AtLeast, "<>",
        lc::geo::Coordinate(0, 0), lc::geo::Coordinate(100, 50), 0.0, 0.0,
        std::make_shared<lc::meta::Layer>());
}

lc::entity::DimLinear_CSPtr asLinear(const lc::entity::CADEntity_CSPtr& entity) {
    return std::dynamic_pointer_cast<const lc::entity::DimLinear>(entity);
}

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimLinearTest, RotatingADimensionTurnsItsLine) {
    const auto rotated = asLinear(horizontal()->rotate({0, 0}, M_PI / 2));

    ASSERT_NE(rotated, nullptr);
    EXPECT_NEAR(rotated->angle(), M_PI / 2, 1e-12);
    EXPECT_NEAR(rotated->definitionPoint3().x(), -50, 1e-9);
}

// Mirroring was not implemented and gave a null entity.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimLinearTest, MirroringADimensionMirrorsItsLine) {
    const auto original = horizontal();

    const auto acrossDiagonal = asLinear(original->mirror({0, 0}, {1, 1}));
    ASSERT_NE(acrossDiagonal, nullptr);
    EXPECT_EQ(acrossDiagonal->id(), original->id());
    EXPECT_NEAR(std::sin(acrossDiagonal->angle()), 1, 1e-12) << "horizontal mirrored across y = x is vertical";
    EXPECT_NEAR(acrossDiagonal->definitionPoint3().x(), 50, 1e-9);
    EXPECT_NEAR(acrossDiagonal->definitionPoint3().y(), 100, 1e-9);

    const auto acrossVertical = asLinear(original->mirror({0, 0}, {0, 1}));
    ASSERT_NE(acrossVertical, nullptr);
    EXPECT_NEAR(std::abs(std::cos(acrossVertical->angle())), 1, 1e-12) << "still horizontal";
}

// A uniform scale keeps the angle; stretching one axis turns the line.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimLinearTest, ScalingTurnsTheLineOnlyWhenTheAxesScaleDifferently) {
    const auto rotated = asLinear(horizontal()->rotate({0, 0}, M_PI / 4));
    ASSERT_NE(rotated, nullptr);

    EXPECT_NEAR(asLinear(rotated->scale({0, 0}, {3, 3}))->angle(), M_PI / 4, 1e-12);
    EXPECT_NEAR(asLinear(rotated->scale({0, 0}, {2, 1}))->angle(), std::atan2(1, 2), 1e-12);
}
