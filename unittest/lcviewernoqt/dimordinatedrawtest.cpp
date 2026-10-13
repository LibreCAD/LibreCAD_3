// What LCDimOrdinate::draw asks the painter to do: the leader, and the
// ordinate at the end of it. Runs against RecordingPainter, which records
// lines as given and text in its y-down device space.

#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <cad/meta/layer.h>
#include <cad/primitive/dimordinate.h>

#include <drawitems/lcdimordinate.h>
#include <lcdrawoptions.h>

#include "recordingpainter.h"

using lc::viewer::testing::RecordingPainter;

namespace {

lc::entity::DimOrdinate_CSPtr ordinate(const lc::geo::Coordinate& feature,
                                       const lc::geo::Coordinate& leaderEnd,
                                       bool xType,
                                       lc::TextConst::AttachmentPoint attachmentPoint,
                                       const std::string& explicitValue = "<>") {
    return std::make_shared<lc::entity::DimOrdinate>(
        lc::geo::Coordinate(0, 0), leaderEnd, attachmentPoint, 0.0, 1.0,
        lc::TextConst::LineSpacingStyle::AtLeast, explicitValue,
        feature, leaderEnd, xType, std::make_shared<lc::meta::Layer>());
}

RecordingPainter draw(const lc::entity::DimOrdinate_CSPtr& entity) {
    RecordingPainter painter;
    lc::viewer::LcDrawOptions options;
    lc::viewer::LCDimOrdinate item(entity);
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

}  // namespace

// An X ordinate's leader runs up from the feature, and jogs across to an end
// that is off to the side of it. The text sits above the end, centred on it.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateDrawTest, AnXOrdinateJogsUpToItsTextAboveTheLeader) {
    const auto painter = draw(ordinate({10, 0}, {16, 30}, true,
                                       lc::TextConst::AttachmentPoint::Bottom_center));

    ASSERT_EQ(painter.lines.size(), 3u);
    expectLine(painter.lines[0], 10, 0, 10, 10, "straight up from the feature");
    expectLine(painter.lines[1], 10, 10, 16, 20, "across to the end's line");
    expectLine(painter.lines[2], 16, 20, 16, 30, "straight up to the end");

    ASSERT_EQ(painter.texts.size(), 1u);
    const auto& text = painter.texts.front();
    EXPECT_EQ(text.text, "10.00");
    const double width = text.text.size() * text.fontSize * 0.5;
    EXPECT_NEAR(text.x, 16 - width / 2, 1e-9) << "not centred on the leader";
    EXPECT_GT(-text.y, 30) << "the text is not above the end of the leader";
}

// A Y ordinate's leader runs sideways, straight when its end is level with
// the feature, and the text starts where it stops.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateDrawTest, AYOrdinateRunsSidewaysToItsTextPastTheLeader) {
    const auto painter = draw(ordinate({10, 5}, {40, 5}, false,
                                       lc::TextConst::AttachmentPoint::Middle_left));

    ASSERT_EQ(painter.lines.size(), 1u);
    expectLine(painter.lines[0], 10, 5, 40, 5, "straight across");

    ASSERT_EQ(painter.texts.size(), 1u);
    const auto& text = painter.texts.front();
    EXPECT_EQ(text.text, "5.00");
    EXPECT_NEAR(text.x, 40, 1e-9) << "the text does not start at the end of the leader";
    EXPECT_LT(-text.y, 5) << "the baseline is not below the middle of the text";
    EXPECT_GT(-text.y + text.fontSize, 5) << "the text is not level with the leader";
}

// Left of the datum the ordinate is negative, and shown without the sign; an
// explicit value takes it in place of "<>".
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DimOrdinateDrawTest, TheOrdinateIsShownWithoutItsSign) {
    EXPECT_EQ(draw(ordinate({-7, 3}, {-7, 20}, true,
                            lc::TextConst::AttachmentPoint::Bottom_center)).texts.front().text,
              "7.00");
    EXPECT_EQ(draw(ordinate({-7, 3}, {-7, 20}, true,
                            lc::TextConst::AttachmentPoint::Bottom_center, "X=<>")).texts.front().text,
              "X=7.00");
}
