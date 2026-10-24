// What LCDimLinear::draw asks the painter to do: a dimension line along the
// dimension's own angle, through its definition point, between the feet of
// the two extension lines. Runs against RecordingPainter, which records lines
// as given.
//
// The orientation used to be guessed from where the text sat, after sorting
// the two points by their distance from the origin, so the cases below that
// sit left of or below the origin drew the wrong orientation, or none.

#include <cmath>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <cad/meta/layer.h>
#include <cad/primitive/dimlinear.h>

#include <drawitems/lcdimlinear.h>
#include <lcdrawoptions.h>

#include "recordingpainter.h"

using lc::viewer::testing::RecordingPainter;

namespace {

lc::entity::DimLinear_CSPtr linear(const lc::geo::Coordinate& first,
                                   const lc::geo::Coordinate& second,
                                   const lc::geo::Coordinate& onLine,
                                   double angle) {
    return std::make_shared<lc::entity::DimLinear>(
        onLine, onLine, lc::TextConst::AttachmentPoint::Middle_center, 0.0, 1.0,
        lc::TextConst::LineSpacingStyle::AtLeast, "<>",
        first, second, angle, 0.0, std::make_shared<lc::meta::Layer>());
}

RecordingPainter draw(const lc::entity::DimLinear_CSPtr& entity) {
    RecordingPainter painter;
    lc::viewer::LcDrawOptions options;
    lc::viewer::LCDimLinear item(entity);
    item.draw(painter, options, lc::geo::Area());
    return painter;
}

void expectLine(const lc::viewer::testing::DrawnLine& line,
                double x1, double y1, double x2, double y2, const char* what) {
    EXPECT_NEAR(line.x1, x1, 1e-9) << what;
    EXPECT_NEAR(line.y1, y1, 1e-9) << what;
    EXPECT_NEAR(line.x2, x2, 1e-9) << what;
    EXPECT_NEAR(line.y2, y2, 1e-9) << what;
}

/// The value drawn: the first text, the measurement.
std::string valueOf(const RecordingPainter& painter) {
    return painter.texts.empty() ? std::string() : painter.texts.front().text;
}

}  // namespace

// The extension lines run from the points to the dimension line, and the
// dimension line joins their feet; the arrowheads are drawn after these.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimLinearDrawTest, AHorizontalDimensionBetweenItsPointsMeasuresAcross) {
    const auto painter = draw(linear({0, 0}, {100, 50}, {100, 25}, 0.0));

    ASSERT_GE(painter.lines.size(), 3u);
    expectLine(painter.lines[0], 100, 50, 100, 25, "second extension line");
    expectLine(painter.lines[1], 100, 25, 0, 25, "dimension line");
    expectLine(painter.lines[2], 0, 25, 0, 0, "first extension line");
    EXPECT_EQ(valueOf(painter), "100.00");
}

// The same two points and the same place between them, measured up instead:
// the angle, not where the line sits, decides.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimLinearDrawTest, AVerticalDimensionBetweenItsPointsMeasuresUp) {
    const auto painter = draw(linear({0, 0}, {100, 50}, {50, 50}, M_PI / 2));

    ASSERT_GE(painter.lines.size(), 3u);
    expectLine(painter.lines[0], 100, 50, 50, 50, "second extension line");
    expectLine(painter.lines[1], 50, 50, 50, 0, "dimension line");
    expectLine(painter.lines[2], 50, 0, 0, 0, "first extension line");
    EXPECT_EQ(valueOf(painter), "50.00");
}

// Two points left of the origin drew no dimension line and said "0.00".
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimLinearDrawTest, WhereThePointsAreDoesNotMatter) {
    auto painter = draw(linear({-100, 0}, {-20, 0}, {-20, 30}, 0.0));
    ASSERT_GE(painter.lines.size(), 3u);
    expectLine(painter.lines[1], -20, 30, -100, 30, "level pair left of the origin");
    EXPECT_EQ(valueOf(painter), "80.00");

    painter = draw(linear({-100, -50}, {0, 0}, {0, 20}, 0.0));
    ASSERT_GE(painter.lines.size(), 3u);
    expectLine(painter.lines[1], 0, 20, -100, 20, "pair below the origin, measured across");
    EXPECT_EQ(valueOf(painter), "100.00");

    painter = draw(linear({20, 300}, {120, 10}, {120, 320}, 0.0));
    EXPECT_EQ(valueOf(painter), "100.00") << "a pair whose first point is the further from the origin";
}

// Any angle: what is measured is the distance between the points along it.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimLinearDrawTest, ARotatedDimensionMeasuresAlongItsAngle) {
    const auto painter = draw(linear({0, 0}, {100, 0}, {100, 0}, M_PI / 6));

    ASSERT_GE(painter.lines.size(), 3u);
    const auto& dimensionLine = painter.lines[1];
    EXPECT_NEAR(std::atan2(dimensionLine.y1 - dimensionLine.y2, dimensionLine.x1 - dimensionLine.x2), M_PI / 6, 1e-9);
    EXPECT_NEAR(std::hypot(dimensionLine.x1 - dimensionLine.x2, dimensionLine.y1 - dimensionLine.y2),
                100 * std::cos(M_PI / 6), 1e-9);
    EXPECT_EQ(valueOf(painter), "86.60");
}

// Points closer together than the text is long get their arrows outside.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimLinearDrawTest, AShortDimensionPutsItsArrowsOutside) {
    const auto painter = draw(linear({0, 0}, {4, 0}, {4, 10}, 0.0));

    ASSERT_GE(painter.lines.size(), 4u);
    expectLine(painter.lines[0], 4, 0, 4, 10, "second extension line");
    expectLine(painter.lines[1], 4, 10, 24, 10, "tail beyond the second foot");
    expectLine(painter.lines[2], -20, 10, 0, 10, "tail beyond the first foot");
    expectLine(painter.lines[3], 0, 10, 0, 0, "first extension line");
    EXPECT_EQ(valueOf(painter), "4.00");
}
