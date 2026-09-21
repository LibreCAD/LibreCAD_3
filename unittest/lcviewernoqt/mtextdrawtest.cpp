// What LCVMText::draw actually asks the painter to do.
//
// The layout arithmetic has its own test; this is the other half -- whether
// draw() hands that arithmetic the right arguments, and puts the lines where
// it says. It runs against RecordingPainter, which draws nothing and
// remembers the transform every call landed under.

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <cad/meta/layer.h>
#include <cad/primitive/mtext.h>
#include <cad/primitive/textconst.h>

#include <drawitems/lcvmtext.h>
#include <lcdrawoptions.h>

#include "recordingpainter.h"

using lc::viewer::testing::RecordingPainter;

namespace {

constexpr double kHeight = 10.0;

lc::entity::MText_CSPtr mtext(const std::string& text,
                              double lineSpacingFactor,
                              double angle = 0.0,
                              lc::TextConst::VAlign valign = lc::TextConst::VAlign::VATop,
                              lc::TextConst::HAlign halign = lc::TextConst::HAlign::HALeft) {
    return std::make_shared<lc::entity::MText>(
        lc::geo::Coordinate(0.0, 0.0, 0.0), text, kHeight, angle, "STANDARD",
        lc::TextConst::DrawingDirection::None, halign, valign,
        /*underlined=*/false, /*strikethrough=*/false, /*bold=*/false, /*italic=*/false,
        /*width=*/0.0, lc::TextConst::MTextDrawingDirection::ByStyle,
        lineSpacingFactor, lc::TextConst::LineSpacingStyle::AtLeast,
        std::make_shared<lc::meta::Layer>());
}

std::vector<lc::viewer::testing::DrawnText> drawLines(
    const lc::entity::MText_CSPtr& entity) {
    RecordingPainter painter;
    lc::viewer::LcDrawOptions options;
    lc::viewer::LCVMText item(entity);
    item.draw(painter, options, lc::geo::Area());
    return painter.texts;
}

}  // namespace

// Every line is drawn, in order, once.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextDrawTest, EachLineIsDrawnOnce) {
    const auto drawn = drawLines(mtext("one\ntwo\nthree", 1.0));

    ASSERT_EQ(drawn.size(), 3u);
    EXPECT_EQ(drawn[0].text, "one");
    EXPECT_EQ(drawn[1].text, "two");
    EXPECT_EQ(drawn[2].text, "three");
}

// The size is set before the lines are measured, and it is the text's own
// height. Measuring at the painter's default is what made every alignment
// offset wrong by the ratio between the two.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextDrawTest, TheLinesAreDrawnAtTheTextsOwnHeight) {
    const auto drawn = drawLines(mtext("one\ntwo", 1.0));

    ASSERT_EQ(drawn.size(), 2u);
    EXPECT_DOUBLE_EQ(drawn[0].fontSize, kHeight);
    EXPECT_DOUBLE_EQ(drawn[1].fontSize, kHeight);
}

// Horizontal alignment is per line, from that line's own width -- issue #361's
// opening complaint. The short line and the long line cannot share an offset.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextDrawTest, EachLineIsAlignedOnItsOwnWidth) {
    const auto drawn = drawLines(mtext("i\nlonger line", 1.0, 0.0,
                                       lc::TextConst::VAlign::VATop,
                                       lc::TextConst::HAlign::HARight));

    ASSERT_EQ(drawn.size(), 2u);
    // RecordingPainter measures a string as length * height/2.
    EXPECT_NEAR(drawn[0].x, -1.0 * kHeight * 0.5, 1e-9);
    EXPECT_NEAR(drawn[1].x, -11.0 * kHeight * 0.5, 1e-9);
    EXPECT_GT(drawn[0].x, drawn[1].x)
        << "both lines were right-aligned on the same width";
}
