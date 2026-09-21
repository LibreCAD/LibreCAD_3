// Where the lines of a multi-line text block go.
//
// LcPainter has sixty-one pure virtuals and there is no mock, so the only way
// to test any of this is to keep the arithmetic out of the painter's way.
// These are measurements in, offsets out.

#include <gtest/gtest.h>

#include <vector>

#include <cad/primitive/textconst.h>
#include <drawitems/mtextlayout.h>

using lc::viewer::layoutTextLines;
using lc::viewer::mtextLinePitch;
using lc::viewer::splitTextLines;

namespace {

constexpr double kHeight = 10.0;
constexpr int kLeft = lc::TextConst::HALeft;
constexpr int kCenter = lc::TextConst::HACenter;
constexpr int kRight = lc::TextConst::HARight;
constexpr int kTop = lc::TextConst::VATop;
constexpr int kMiddle = lc::TextConst::VAMiddle;
constexpr int kBottom = lc::TextConst::VABottom;

const std::vector<double> kThreeLines = {30.0, 10.0, 20.0};

}  // namespace

// The defect issue #361 opens with: every line was aligned using one width
// measured from the whole newline-joined string.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextLayoutTest, HorizontalAlignmentIsPerLine) {
    const auto left = layoutTextLines(kLeft, kTop, kHeight, 1.0, kThreeLines);
    ASSERT_EQ(left.size(), 3u);
    EXPECT_DOUBLE_EQ(left[0].x, 0.0);
    EXPECT_DOUBLE_EQ(left[1].x, 0.0);
    EXPECT_DOUBLE_EQ(left[2].x, 0.0);

    const auto centre = layoutTextLines(kCenter, kTop, kHeight, 1.0, kThreeLines);
    EXPECT_DOUBLE_EQ(centre[0].x, -15.0) << "each line is centred on its own width";
    EXPECT_DOUBLE_EQ(centre[1].x, -5.0);
    EXPECT_DOUBLE_EQ(centre[2].x, -10.0);

    const auto right = layoutTextLines(kRight, kTop, kHeight, 1.0, kThreeLines);
    EXPECT_DOUBLE_EQ(right[0].x, -30.0);
    EXPECT_DOUBLE_EQ(right[1].x, -10.0);
    EXPECT_DOUBLE_EQ(right[2].x, -20.0);
}

// The anchor is the block, not the first line. Anchoring the first line put a
// middle- or bottom-aligned block (n-1) pitches too low.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextLayoutTest, VerticalAlignmentAnchorsTheBlock) {
    const double pitch = mtextLinePitch(kHeight, 1.0);
    const double drop = pitch * 2.0;  // three lines

    const auto top = layoutTextLines(kLeft, kTop, kHeight, 1.0, kThreeLines);
    const auto middle = layoutTextLines(kLeft, kMiddle, kHeight, 1.0, kThreeLines);
    const auto bottom = layoutTextLines(kLeft, kBottom, kHeight, 1.0, kThreeLines);

    // Top: the block hangs below the anchor, so the first line is where a
    // single-line block would be.
    EXPECT_DOUBLE_EQ(top[0].y, -kHeight + kHeight * 0.2);
    // Bottom: the LAST line sits where the first line does for top-ish
    // alignment, so the block is lifted by its whole drop.
    EXPECT_DOUBLE_EQ(bottom[2].y - bottom[0].y, drop);
    EXPECT_DOUBLE_EQ(bottom[0].y, kHeight * 0.2 - drop);
    // Middle: half of it.
    EXPECT_DOUBLE_EQ(middle[0].y, -kHeight / 2.0 + kHeight * 0.2 - drop / 2.0);
}

// A one-line block must land exactly where it always did, or every existing
// drawing shifts the moment this lands.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextLayoutTest, SingleLineReproducesTheOldOffsets) {
    const std::vector<double> one = {42.0};
    EXPECT_DOUBLE_EQ(layoutTextLines(kLeft, kTop, kHeight, 1.0, one)[0].y,
                     -kHeight + kHeight * 0.2);
    EXPECT_DOUBLE_EQ(layoutTextLines(kLeft, kMiddle, kHeight, 1.0, one)[0].y,
                     -kHeight / 2.0 + kHeight * 0.2);
    EXPECT_DOUBLE_EQ(layoutTextLines(kLeft, kBottom, kHeight, 1.0, one)[0].y,
                     kHeight * 0.2);
    EXPECT_DOUBLE_EQ(layoutTextLines(kCenter, kTop, kHeight, 1.0, one)[0].x, -21.0);
    EXPECT_DOUBLE_EQ(layoutTextLines(kRight, kTop, kHeight, 1.0, one)[0].x, -42.0);
}

// DXF spaces MTEXT lines at 5/3 of the height, not at the height.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextLayoutTest, LinePitchFollowsTheDxfRule) {
    EXPECT_DOUBLE_EQ(mtextLinePitch(10.0, 1.0), 10.0 * 5.0 / 3.0);
    EXPECT_DOUBLE_EQ(mtextLinePitch(10.0, 2.0), 10.0 * 5.0 / 3.0 * 2.0);
    EXPECT_DOUBLE_EQ(mtextLinePitch(10.0, 0.0), 10.0 * 5.0 / 3.0)
        << "a zero factor is absent, not a collapsed block";

    const auto spaced = layoutTextLines(kLeft, kTop, kHeight, 2.0, kThreeLines);
    EXPECT_DOUBLE_EQ(spaced[1].y - spaced[0].y, mtextLinePitch(kHeight, 2.0));
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextLayoutTest, LinesAreEvenlySpacedRegardlessOfAlignment) {
    for (const int valign : {kTop, kMiddle, kBottom}) {
        const auto o = layoutTextLines(kLeft, valign, kHeight, 1.0, kThreeLines);
        EXPECT_DOUBLE_EQ(o[1].y - o[0].y, o[2].y - o[1].y) << "valign " << valign;
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextLayoutTest, SplittingKeepsEmptyLines) {
    EXPECT_EQ(splitTextLines("a\nb").size(), 2u);
    EXPECT_EQ(splitTextLines("a\n\nb").size(), 3u) << "a blank line is a blank line";
    EXPECT_EQ(splitTextLines("").size(), 1u);
    EXPECT_EQ(splitTextLines("trailing\n").size(), 2u);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextLayoutTest, NoLinesIsNotACrash) {
    EXPECT_TRUE(layoutTextLines(kLeft, kTop, kHeight, 1.0, {}).empty());
}
