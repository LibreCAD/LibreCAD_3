#include "lcvmtext.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include <cad/primitive/textconst.h>
#include "mtextlayout.h"

#include <cstddef>
#include <string>
#include <vector>

using namespace lc::viewer;

LCVMText::LCVMText(const lc::entity::MText_CSPtr& mtext) :
    LCVDrawItem(mtext, true),
    _mtext(mtext) {
}

/**
* TODO items for text:
* 1) Test multi-line text rendering
* 2) Accept color codes during text rendering in the form of \Cx (x is the index color)
* 3) Font selection
* For testing there is a lua script
*
* We can increase performance if we pre-calculate some values and cache them so that we
* don't have to keep calling text_extends and do some of the calculations
* also we shouldn't draw text smaller then XX pixels when rendered on screen.
*/
void LCVMText::draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const {
    setFontFace(painter);

    // Set the size BEFORE measuring. text_extends scales by the painter's
    // current text height, and the renderer resets that to a default after
    // every stroke -- so every measurement here used to be taken at that
    // default rather than at the text's own height, and every alignment
    // offset was wrong by the ratio between them. LCVText has always done it
    // in this order.
    painter.font_size(_mtext->height(), false);

    const std::vector<std::string> lines = splitTextLines(_mtext->text_value());

    std::vector<double> widths;
    widths.reserve(lines.size());
    for (const std::string& line : lines) {
        widths.push_back(painter.text_extends(line.c_str()).width);
    }

    const std::vector<TextLineOffset> offsets = layoutTextLines(
        _mtext->halign(), _mtext->valign(), _mtext->height(), 1.0, widths);

    for (std::size_t i = 0; i < lines.size(); i++) {
        // One transform per line, and the line advance goes INSIDE the
        // rotation. It used to be folded into the translation that precedes
        // the rotate, so a rotated block stacked its lines along the unrotated
        // axis: at 90 degrees they printed on top of each other.
        painter.save();
        painter.translate(_mtext->insertion_point().x(), -_mtext->insertion_point().y());
        painter.rotate(-_mtext->angle());
        painter.translate(offsets[i].x, offsets[i].y);

        painter.move_to(0.0, 0.0);
        painter.text(lines[i].c_str());
        painter.stroke();

        // The rules belong to the line they cross, at that line's width. They
        // used to be drawn at the first line's position, spanning the whole
        // block's measured width, with the underline offset a hardcoded 8
        // drawing units -- an absolute distance in a drawing whose text may be
        // 2.5mm or 100m tall.
        if (_mtext->strikethrough()) {
            painter.move_to(0.0, -_mtext->height() / 3.0);
            painter.line_to(widths[i], -_mtext->height() / 3.0);
            painter.stroke();
        }

        if (_mtext->underlined()) {
            painter.move_to(0.0, _mtext->height() / 5.0);
            painter.line_to(widths[i], _mtext->height() / 5.0);
            painter.stroke();
        }

        painter.restore();
    }
}

void LCVMText::setFontFace(LcPainter& painter) const {
    if (_mtext->style() != "" && _mtext->style() != "STANDARD") {
        if (_mtext->bold() && _mtext->italic()) {
            painter.select_font_face(_mtext->style().c_str(), "BOLD_ITALIC");
        }
        else if (_mtext->bold()) {
            painter.select_font_face(_mtext->style().c_str(), "BOLD");
        }
        else if (_mtext->italic()) {
            painter.select_font_face(_mtext->style().c_str(), "ITALIC");
        }
        else {
            painter.select_font_face(_mtext->style().c_str(), "REGULAR");
        }
    }
}

lc::entity::CADEntity_CSPtr LCVMText::entity() const {
    return _mtext;
}


