#include "lcvtext.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include <cad/primitive/textconst.h>

using namespace LCViewer;

LCVText::LCVText(const lc::entity::Text_CSPtr text) :
        LCVDrawItem(text, true),
        _text(text) {
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
void LCVText::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    painter.font_size(_text->height(), false);
    painter.select_font_face("stick3.ttf");

    TextExtends te = painter.text_extends(_text->text_value().c_str());
    double alignX = 0.0;
    double alignY = 0.0;

    //    double alignX, alignY;
    // The idea of height() * .2 is just a average basline offset. Don't this value to seriously,
    // we could get it from font exists but that sounds over exaggerating for the moment.
    switch (_text->valign()) {
        case lc::TextConst::VAMiddle:
            alignX += 0.0;
            alignY += -_text->height() / 2. + (_text->height() * .2);
            break;

        case lc::TextConst::VABottom:
            alignX += 0.0;
            alignY += -_text->height() + (_text->height() * .2);
            break;

        case lc::TextConst::VABaseline:
            alignX += 0.0;
            alignY += 0.0;
            break;

        case lc::TextConst::VATop:
            alignX += 0.0;
            alignY += 0.0 + (_text->height() * .2);
            break;

        default:
            break;
    }

    // Horizontal Align:
    switch (_text->halign()) {
        case lc::TextConst::HALeft:
            alignX += - te.width;
            alignY += 0.;
            break;

        case lc::TextConst::HACenter:
            alignX += - te.width / 2.0;
            alignY += 0.;
            break;

        case lc::TextConst::HAMiddle:
            alignX += - te.width / 2.0;
            alignY += 0.;
            break;

        case lc::TextConst::HARight:
            alignX += 0.;
            alignY += 0.;
            break;

        default:
            break;
    }

    painter.save();
    painter.translate(_text->insertion_point().x(), -_text->insertion_point().y());
    painter.rotate(-_text->angle());
    painter.translate(alignX, -alignY);
    painter.move_to(0., 0.);
    painter.text(_text->text_value().c_str());
    painter.stroke();
    painter.restore();
}

lc::entity::CADEntity_CSPtr LCVText::entity() const {
    return _text;
}


