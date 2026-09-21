#include "textblockbox.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include <cad/primitive/textconst.h>

namespace lc {
namespace entity {

namespace {

/// Count characters, not bytes.
///
/// The box this feeds used to be sized from std::string::size(), so any
/// non-ASCII text inflated it by the length of its UTF-8 encoding -- a line of
/// CJK was measured three times too wide and selected from three times too far
/// away.
std::size_t countCodePoints(const std::string& s) {
    std::size_t n = 0;
    for (const char c : s) {
        if ((static_cast<unsigned char>(c) & 0xC0u) != 0x80u) {
            n++;   // not a continuation byte, so it starts a character
        }
    }
    return n;
}

}  // namespace

geo::Area textBlockBoundingBox(const geo::Coordinate& insertionPoint,
                               const std::string& text,
                               double height,
                               double angle,
                               int halign,
                               int valign,
                               double lineSpacingFactor,
                               bool multiLine) {
    std::size_t lineCount = 1;
    std::size_t widest = 0;

    if (multiLine) {
        std::size_t start = 0;
        std::size_t pos = 0;
        lineCount = 0;
        while ((pos = text.find('\n', start)) != std::string::npos) {
            widest = std::max(widest, countCodePoints(text.substr(start, pos - start)));
            lineCount++;
            start = pos + 1;
        }
        widest = std::max(widest, countCodePoints(text.substr(start)));
        lineCount++;
    } else {
        widest = countCodePoints(text);
    }

    const double width =
        static_cast<double>(widest) * height * TextConst::NominalGlyphWidthRatio;
    // The same pitch the viewer places the lines at, including group 44. A box
    // computed without the factor is up to four times too short for a text
    // that asks for wide spacing, and this box is what the quadtree indexes --
    // so selection, snapping and zoom-to-fit would all stop short of the
    // drawing.
    const double pitch = TextConst::mtextLinePitch(height, lineSpacingFactor);
    const double blockHeight =
        height + pitch * static_cast<double>(lineCount - 1);

    // Where the block sits relative to the anchor. The old box was symmetric
    // about the insertion point in both axes, which is not where text is: it
    // hangs from the alignment it was given.
    double left = 0.0;
    switch (halign) {
    case TextConst::HACenter:
    case TextConst::HAMiddle:
        left = -width / 2.0;
        break;
    case TextConst::HARight:
        left = -width;
        break;
    default:
        left = 0.0;
        break;
    }

    double bottom = 0.0;
    switch (valign) {
    case TextConst::VATop:
        bottom = -blockHeight;
        break;
    case TextConst::VAMiddle:
        bottom = -blockHeight / 2.0;
        break;
    default:   // VABottom, VABaseline
        bottom = 0.0;
        break;
    }

    const double right = left + width;
    const double top = bottom + blockHeight;

    // Rotate the corners and take the axis-aligned box around them. A rotated
    // text used to report the same box as an unrotated one.
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    const double xs[4] = {left, right, right, left};
    const double ys[4] = {bottom, bottom, top, top};

    double minX = 0.0;
    double minY = 0.0;
    double maxX = 0.0;
    double maxY = 0.0;
    for (int i = 0; i < 4; i++) {
        const double x = xs[i] * c - ys[i] * s;
        const double y = xs[i] * s + ys[i] * c;
        if (i == 0) {
            minX = maxX = x;
            minY = maxY = y;
        } else {
            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }
    }

    return geo::Area(
        geo::Coordinate(insertionPoint.x() + minX, insertionPoint.y() + minY),
        geo::Coordinate(insertionPoint.x() + maxX, insertionPoint.y() + maxY));
}

}  // namespace entity
}  // namespace lc
