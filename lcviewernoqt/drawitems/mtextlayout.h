#pragma once

#include <string>
#include <vector>

namespace lc {
namespace viewer {

/**
 * Where each line of a multi-line text block goes.
 *
 * The arithmetic is separated from the drawing for one reason: LcPainter has
 * sixty-one pure virtuals and there is no mock, so anything that touches a
 * painter cannot be unit tested. Everything here is measurements in, offsets
 * out.
 *
 * Coordinates are in the text's own frame, before rotation and before the
 * translation to the insertion point: x grows right, y grows down the block.
 * The caller rotates. That ordering is the fix for rotated multi-line text --
 * folding the line advance into the pre-rotation translation made the lines
 * stack along the unrotated axis and cross each other.
 */
struct TextLineOffset {
    double x{0.0};
    double y{0.0};
};

/**
 * DXF's line pitch for MTEXT.
 *
 * A line is 5/3 of the nominal text height apart, scaled by the line-spacing
 * factor (group 44). The renderer used to advance by the height itself, which
 * packs the lines about 40% too close.
 */
double mtextLinePitch(double height, double lineSpacingFactor);

/**
 * One offset per line, given each line's measured width.
 *
 * Horizontal alignment is per line -- that is the defect issue #361 opens
 * with. Vertical alignment is for the block: a three-line block anchored at
 * its middle sits a line higher than a one-line block does, which is why the
 * line count has to reach this calculation rather than just the height.
 *
 * `halign` and `valign` are lc::TextConst::HAlign / VAlign values, passed as
 * int so this header stays independent of the kernel.
 */
std::vector<TextLineOffset> layoutTextLines(int halign, int valign,
                                            double height,
                                            double lineSpacingFactor,
                                            const std::vector<double>& lineWidths);

/** Split on newlines, keeping empty lines -- a blank line is a blank line. */
std::vector<std::string> splitTextLines(const std::string& text);

}  // namespace viewer
}  // namespace lc
