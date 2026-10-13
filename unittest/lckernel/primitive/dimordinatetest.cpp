// The ordinate dimension: the distance along X or Y from a datum to a feature.

#include <gtest/gtest.h>

#include <cmath>
#include <memory>

#include <cad/builders/dimordinate.h>
#include <cad/meta/block.h>
#include <cad/meta/layer.h>
#include <cad/primitive/dimordinate.h>

using namespace lc;
using namespace entity;

namespace {

meta::Layer_CSPtr layer() {
    static auto layer = std::make_shared<meta::Layer>("0");
    return layer;
}

/// An ordinate of the feature (35, 70) from the datum (10, 20), its leader
/// running up to (35, 90).
DimOrdinate_CSPtr ordinate(bool xType, meta::Block_CSPtr block = nullptr) {
    return std::make_shared<DimOrdinate>(
        geo::Coordinate(10, 20), geo::Coordinate(35, 90),
        TextConst::AttachmentPoint::Bottom_center, 0.0, 1.0,
        TextConst::LineSpacingStyle::AtLeast, "<>",
        geo::Coordinate(35, 70), geo::Coordinate(35, 90), xType,
        layer(), nullptr, block);
}

void expectSamePoint(const geo::Coordinate& actual, const geo::Coordinate& expected, const char* what) {
    EXPECT_NEAR(actual.x(), expected.x(), 1e-9) << what;
    EXPECT_NEAR(actual.y(), expected.y(), 1e-9) << what;
}

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateTest, TheValueIsMeasuredFromTheDatumAlongOneAxis) {
    EXPECT_DOUBLE_EQ(ordinate(true)->value(), 25.0);
    EXPECT_DOUBLE_EQ(ordinate(false)->value(), 50.0);

    // Left of and below the datum, the ordinate is negative.
    auto behind = std::make_shared<DimOrdinate>(
        geo::Coordinate(10, 20), geo::Coordinate(-5, 40),
        TextConst::AttachmentPoint::Bottom_center, 0.0, 1.0,
        TextConst::LineSpacingStyle::AtLeast, "<>",
        geo::Coordinate(-5, 5), geo::Coordinate(-5, 40), true, layer());
    EXPECT_DOUBLE_EQ(behind->value(), -15.0);
    EXPECT_DOUBLE_EQ(std::static_pointer_cast<const DimOrdinate>(
                         behind->setProperties({{"xType", false}}))->value(), -15.0);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateTest, TheBuilderCarriesEveryField) {
    builder::DimOrdinateBuilder builder;
    builder.setLayer(layer());
    builder.setDefinitionPoint({1, 2});
    builder.setMiddleOfText({7, 9});
    builder.setAttachmentPoint(TextConst::AttachmentPoint::Middle_left);
    builder.setTextAngle(0.25);
    builder.setExplicitValue("X <>");
    builder.setFeaturePoint({3, 4});
    builder.setLeaderEndPoint({7, 8});
    builder.setXType(false);

    auto built = builder.build();
    expectSamePoint(built->definitionPoint(), {1, 2}, "datum");
    expectSamePoint(built->middleOfText(), {7, 9}, "text");
    EXPECT_EQ(built->attachmentPoint(), TextConst::AttachmentPoint::Middle_left);
    EXPECT_DOUBLE_EQ(built->textAngle(), 0.25);
    EXPECT_EQ(built->explicitValue(), "X <>");
    expectSamePoint(built->featurePoint(), {3, 4}, "feature");
    expectSamePoint(built->leaderEndPoint(), {7, 8}, "leader end");
    EXPECT_FALSE(built->xType());
    EXPECT_DOUBLE_EQ(built->value(), 2.0);

    // And back again: a builder copied from the entity builds the same one.
    builder::DimOrdinateBuilder copied;
    copied.copy(built);
    auto rebuilt = copied.build();
    EXPECT_EQ(rebuilt->id(), built->id());
    expectSamePoint(rebuilt->definitionPoint(), {1, 2}, "copied datum");
    expectSamePoint(rebuilt->middleOfText(), {7, 9}, "copied text");
    EXPECT_EQ(rebuilt->attachmentPoint(), TextConst::AttachmentPoint::Middle_left);
    EXPECT_EQ(rebuilt->explicitValue(), "X <>");
    expectSamePoint(rebuilt->featurePoint(), {3, 4}, "copied feature");
    expectSamePoint(rebuilt->leaderEndPoint(), {7, 8}, "copied leader end");
    EXPECT_FALSE(rebuilt->xType());
    EXPECT_EQ(rebuilt->layer(), layer());
}

// The leader's direction picks the axis, as in AutoCAD, and the text goes on
// the far side of the leader's end.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateTest, TheLeaderDirectionPicksTheAxis) {
    const geo::Coordinate feature(10, 10);
    const struct {
        geo::Coordinate leaderEnd;
        bool xType;
        TextConst::AttachmentPoint attachmentPoint;
        const char* what;
    } cases[] = {
        {{12, 40}, true, TextConst::AttachmentPoint::Bottom_center, "up"},
        {{8, -30}, true, TextConst::AttachmentPoint::Top_center, "down"},
        {{40, 12}, false, TextConst::AttachmentPoint::Middle_left, "right"},
        {{-20, 8}, false, TextConst::AttachmentPoint::Middle_right, "left"},
        {{30, 30}, true, TextConst::AttachmentPoint::Bottom_center, "diagonal"},
    };

    for (const auto& c : cases) {
        builder::DimOrdinateBuilder builder;
        builder.setLayer(layer());
        builder.setDefinitionPoint({1, 2});
        builder.dimAuto(feature, c.leaderEnd);

        EXPECT_EQ(builder::DimOrdinateBuilder::measuresX(feature, c.leaderEnd), c.xType) << c.what;
        auto built = builder.build();
        EXPECT_EQ(built->xType(), c.xType) << c.what;
        EXPECT_EQ(built->attachmentPoint(), c.attachmentPoint) << c.what;
        expectSamePoint(built->featurePoint(), feature, c.what);
        expectSamePoint(built->leaderEndPoint(), c.leaderEnd, c.what);
        expectSamePoint(built->middleOfText(), c.leaderEnd, c.what);
        expectSamePoint(built->definitionPoint(), {1, 2}, "dimAuto leaves the datum alone");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateTest, MoveKeepsTheIdAndMovesEveryPoint) {
    auto original = ordinate(true);
    auto moved = std::dynamic_pointer_cast<const DimOrdinate>(original->move({5, -3}));
    ASSERT_NE(moved, nullptr);

    EXPECT_EQ(moved->id(), original->id());
    expectSamePoint(moved->definitionPoint(), {15, 17}, "datum");
    expectSamePoint(moved->middleOfText(), {40, 87}, "text");
    expectSamePoint(moved->featurePoint(), {40, 67}, "feature");
    expectSamePoint(moved->leaderEndPoint(), {40, 87}, "leader end");
    EXPECT_TRUE(moved->xType());
    EXPECT_DOUBLE_EQ(moved->value(), original->value());
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateTest, CopyGetsANewIdAndKeepsTheBlock) {
    auto block = std::make_shared<meta::Block>("HOLDER", geo::Coordinate(0, 0));
    auto original = ordinate(false, block);
    auto copied = std::dynamic_pointer_cast<const DimOrdinate>(original->copy({1, 1}));
    ASSERT_NE(copied, nullptr);

    EXPECT_NE(copied->id(), original->id());
    EXPECT_EQ(copied->block(), block);
    EXPECT_EQ(copied->layer(), original->layer());
    expectSamePoint(copied->featurePoint(), {36, 71}, "feature");
    EXPECT_FALSE(copied->xType());
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateTest, ModifyKeepsTheIdAndTheGeometry) {
    auto original = ordinate(true);
    auto block = std::make_shared<meta::Block>("HOLDER", geo::Coordinate(0, 0));
    auto other = std::make_shared<meta::Layer>("other");
    auto modified = std::dynamic_pointer_cast<const DimOrdinate>(original->modify(other, nullptr, block));
    ASSERT_NE(modified, nullptr);

    EXPECT_EQ(modified->id(), original->id());
    EXPECT_EQ(modified->layer(), other);
    EXPECT_EQ(modified->block(), block);
    expectSamePoint(modified->featurePoint(), original->featurePoint(), "feature");
    expectSamePoint(modified->leaderEndPoint(), original->leaderEndPoint(), "leader end");
    EXPECT_TRUE(modified->xType());
}

// The datum is usually the drawing's origin, a long way from anything the
// dimension draws. A box around it would make every ordinate a hit for a click
// anywhere between the two.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateTest, TheBoundingBoxLeavesOutAFarDatum) {
    auto far = std::make_shared<DimOrdinate>(
        geo::Coordinate(0, 0), geo::Coordinate(1020, 1015),
        TextConst::AttachmentPoint::Middle_left, 0.0, 1.0,
        TextConst::LineSpacingStyle::AtLeast, "<>",
        geo::Coordinate(1000, 1000), geo::Coordinate(1010, 1030), true, layer());

    const auto box = far->boundingBox();
    expectSamePoint(box.minP(), {1000, 1000}, "lower left");
    expectSamePoint(box.maxP(), {1020, 1030}, "upper right");
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateTest, PropertiesRoundTrip) {
    auto original = ordinate(true);
    const auto properties = original->availableProperties();

    ASSERT_EQ(properties.count("featurePoint"), 1u);
    ASSERT_EQ(properties.count("leaderEndPoint"), 1u);
    ASSERT_EQ(properties.count("xType"), 1u);
    ASSERT_EQ(properties.count("definitionPoint"), 1u);
    EXPECT_TRUE(boost::get<bool>(properties.at("xType")));
    expectSamePoint(boost::get<geo::Coordinate>(properties.at("featurePoint")), {35, 70}, "feature");
    expectSamePoint(boost::get<geo::Coordinate>(properties.at("leaderEndPoint")), {35, 90}, "leader end");

    // Setting back what was read changes nothing.
    auto same = std::dynamic_pointer_cast<const DimOrdinate>(original->setProperties(properties));
    ASSERT_NE(same, nullptr);
    EXPECT_EQ(same->id(), original->id());
    EXPECT_TRUE(same->xType());
    expectSamePoint(same->featurePoint(), {35, 70}, "unchanged feature");
    EXPECT_EQ(same->explicitValue(), "<>");

    // And each one can be changed on its own.
    auto changed = std::dynamic_pointer_cast<const DimOrdinate>(original->setProperties({
        {"xType", false},
        {"featurePoint", geo::Coordinate(11, 42)},
        {"leaderEndPoint", geo::Coordinate(50, 42)},
        {"explicitValue", std::string("Y <>")},
    }));
    ASSERT_NE(changed, nullptr);
    EXPECT_EQ(changed->id(), original->id());
    EXPECT_FALSE(changed->xType());
    expectSamePoint(changed->featurePoint(), {11, 42}, "changed feature");
    expectSamePoint(changed->leaderEndPoint(), {50, 42}, "changed leader end");
    expectSamePoint(changed->definitionPoint(), {10, 20}, "the datum stays");
    EXPECT_EQ(changed->explicitValue(), "Y <>");
    EXPECT_DOUBLE_EQ(changed->value(), 22.0);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateTest, DragPointsRoundTrip) {
    auto original = ordinate(true);
    auto points = original->dragPoints();
    ASSERT_EQ(points.size(), 4u);
    expectSamePoint(points[0], {10, 20}, "datum");
    expectSamePoint(points[2], {35, 70}, "feature");

    points[2] = geo::Coordinate(45, 70);
    auto dragged = std::dynamic_pointer_cast<const DimOrdinate>(original->setDragPoints(points));
    ASSERT_NE(dragged, nullptr);
    EXPECT_EQ(dragged->id(), original->id());
    EXPECT_DOUBLE_EQ(dragged->value(), 35.0);
    EXPECT_TRUE(dragged->xType());
}

// Mirroring and rotating move every point, the datum included, and keep the
// axis the dimension measures.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateTest, MirrorRotateAndScaleTransformEveryPoint) {
    auto original = ordinate(true);

    // Across the vertical line x = 0.
    auto mirrored = std::dynamic_pointer_cast<const DimOrdinate>(original->mirror({0, 0}, {0, 1}));
    ASSERT_NE(mirrored, nullptr);
    EXPECT_EQ(mirrored->id(), original->id());
    expectSamePoint(mirrored->definitionPoint(), {-10, 20}, "mirrored datum");
    expectSamePoint(mirrored->featurePoint(), {-35, 70}, "mirrored feature");
    expectSamePoint(mirrored->leaderEndPoint(), {-35, 90}, "mirrored leader end");
    expectSamePoint(mirrored->middleOfText(), {-35, 90}, "mirrored text");
    EXPECT_TRUE(mirrored->xType());
    EXPECT_DOUBLE_EQ(mirrored->value(), -25.0);

    // A quarter turn about the datum.
    auto rotated = std::dynamic_pointer_cast<const DimOrdinate>(original->rotate({10, 20}, M_PI / 2));
    ASSERT_NE(rotated, nullptr);
    EXPECT_EQ(rotated->id(), original->id());
    expectSamePoint(rotated->definitionPoint(), {10, 20}, "rotated datum");
    expectSamePoint(rotated->featurePoint(), {-40, 45}, "rotated feature");
    expectSamePoint(rotated->leaderEndPoint(), {-60, 45}, "rotated leader end");
    EXPECT_TRUE(rotated->xType());

    auto scaled = std::dynamic_pointer_cast<const DimOrdinate>(original->scale({10, 20}, {2, 2}));
    ASSERT_NE(scaled, nullptr);
    EXPECT_EQ(scaled->id(), original->id());
    expectSamePoint(scaled->featurePoint(), {60, 120}, "scaled feature");
    EXPECT_DOUBLE_EQ(scaled->value(), 50.0);
}
