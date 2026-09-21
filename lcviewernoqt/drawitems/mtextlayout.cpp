#include "mtextlayout.h"

#include <cstddef>

#include <cad/primitive/textconst.h>

namespace lc {
namespace viewer {

namespace {

/// DXF puts one line of MTEXT 5/3 of the nominal height below the last.
constexpr double kMTextLinePitchRatio = 5.0 / 3.0;

/// The descender allowance the single-line renderer has always applied.
/// Preserved exactly so no existing single-line MText shifts on screen.
constexpr double kDescenderAllowance = 0.2;

}  // namespace

double mtextLinePitch(double height, double lineSpacingFactor) {
    const double factor = lineSpacingFactor > 0.0 ? lineSpacingFactor : 1.0;
    return height * kMTextLinePitchRatio * factor;
}

std::vector<std::string> splitTextLines(const std::string& text) {
    std::vector<std::string> lines;
    std::string::size_type start = 0;
    std::string::size_type pos = 0;

    while ((pos = text.find('\n', start)) != std::string::npos) {
        lines.push_back(text.substr(start, pos - start));
        start = pos + 1;
    }
    lines.push_back(text.substr(start));

    return lines;
}

std::vector<TextLineOffset> layoutTextLines(int halign, int valign,
                                            double height,
                                            double lineSpacingFactor,
                                            const std::vector<double>& lineWidths) {
    std::vector<TextLineOffset> offsets;
    offsets.reserve(lineWidths.size());
    if (lineWidths.empty()) {
        return offsets;
    }

    const double pitch = mtextLinePitch(height, lineSpacingFactor);
    const double blockDrop = pitch * static_cast<double>(lineWidths.size() - 1);

    // Vertical: the anchor is the block, not the first line. Anchoring the
    // first line is what made a middle- or bottom-aligned multi-line MText
    // sit (n-1) pitches too low -- the error grows with every line added,
    // which is why it was never noticed on the single-line case.
    double blockY = 0.0;
    switch (valign) {
    case lc::TextConst::VATop:
        blockY = -height + (height * kDescenderAllowance);
        break;
    case lc::TextConst::VAMiddle:
        blockY = -height / 2.0 + (height * kDescenderAllowance) - blockDrop / 2.0;
        break;
    case lc::TextConst::VABottom:
        blockY = (height * kDescenderAllowance) - blockDrop;
        break;
    case lc::TextConst::VABaseline:
    default:
        blockY = -blockDrop;
        break;
    }

    for (std::size_t i = 0; i < lineWidths.size(); i++) {
        // Horizontal: per line, from that line's own measured width. Using one
        // width for every line is issue #361's opening complaint; the width in
        // question was the whole newline-joined string measured as a single
        // run, so the block was displaced by a distance that had nothing to do
        // with any line in it.
        double x = 0.0;
        switch (halign) {
        case lc::TextConst::HACenter:
        case lc::TextConst::HAMiddle:
            x = -lineWidths[i] / 2.0;
            break;
        case lc::TextConst::HARight:
            x = -lineWidths[i];
            break;
        case lc::TextConst::HALeft:
        default:
            x = 0.0;
            break;
        }

        TextLineOffset offset;
        offset.x = x;
        offset.y = blockY + pitch * static_cast<double>(i);
        offsets.push_back(offset);
    }

    return offsets;
}

}  // namespace viewer
}  // namespace lc
