#include "lcvmtext.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include <cad/primitive/textconst.h>

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
    TextExtends te = painter.text_extends(_mtext->text_value().c_str());
    double alignX = 0.0;
    double alignY = 0.0;
    setAlignment(alignX, alignY, te.width);

    std::string textval = _mtext->text_value();
    std::vector<std::string> lines;
    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = textval.find('\n', prev)) != std::string::npos)
    {
        lines.push_back(textval.substr(prev, pos - prev));
        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    lines.push_back(textval.substr(prev));

    double heightOffset = 0;
    for (const std::string& line : lines) {
        painter.font_size(_mtext->height(), false);
        painter.save();
        painter.translate(_mtext->insertion_point().x(), -_mtext->insertion_point().y() + heightOffset);
        painter.rotate(-_mtext->angle());
        painter.translate(alignX, -alignY);
        painter.move_to(0., 0.);
        painter.text(line.c_str());
        painter.stroke();
        painter.restore();

        if (_mtext->strikethrough()) {
            painter.save();
            painter.translate(_mtext->insertion_point().x(), -_mtext->insertion_point().y() - (te.height / 6.0));
            painter.rotate(-_mtext->angle());
            painter.translate(alignX, -alignY);
            painter.move_to(0., 0);
            painter.line_to(te.width, 0);
            painter.stroke();
            painter.restore();
        }

        if (_mtext->underlined()) {
            painter.save();
            painter.translate(_mtext->insertion_point().x(), -_mtext->insertion_point().y() + 8.0);
            painter.rotate(-_mtext->angle());
            painter.translate(alignX, -alignY);
            painter.move_to(0., 0);
            painter.line_to(te.width, 0);
            painter.stroke();
            painter.restore();
        }

        heightOffset += _mtext->height();
    }
}

void LCVMText::setAlignment(double& alignX, double& alignY, const double textExtendsWidth) const {
    switch (_mtext->valign()) {
    case lc::TextConst::VAMiddle:
        alignX += 0.0;
        alignY += -_mtext->height() / 2. + (_mtext->height() * .2);
        break;

    case lc::TextConst::VABottom:
        alignX += 0.0;
        alignY += 0.0 + (_mtext->height() * .2);
        break;

    case lc::TextConst::VABaseline:
        alignX += 0.0;
        alignY += 0.0;
        break;

    case lc::TextConst::VATop:
        alignX += 0.0;
        alignY += -_mtext->height() + (_mtext->height() * .2);
        break;

    default:
        break;
    }

    // Horizontal Align:
    switch (_mtext->halign()) {
    case lc::TextConst::HALeft:
        alignX += 0;
        alignY += 0.;
        break;

    case lc::TextConst::HACenter:
        alignX += -textExtendsWidth / 2.0;
        alignY += 0.;
        break;

    case lc::TextConst::HAMiddle:
        alignX += -textExtendsWidth / 2.0;
        alignY += 0.;
        break;

    case lc::TextConst::HARight:
        alignX += -textExtendsWidth;
        alignY += 0.;
        break;

    default:
        break;
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


