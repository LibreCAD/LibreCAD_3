#include "lcvtext.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include <cad/primitive/textconst.h>

using namespace LCViewer;

LCVText::LCVText(const lc::entity::Text_CSPtr Text) : LCVDrawItem(true), lc::entity::Text(Text, true) {
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
    bool modified = false;
    painter.font_size(height(), false);
    painter.select_font_face("stick3.ttf");

    TextExtends te = painter.text_extends(text_value().c_str());
    double alignX = 0.0;
    double alignY = 0.0;

    //    double alignX, alignY;
    // The idea of height() * .2 is just a average basline offset. Don't this value to seriously,
    // we could get it from font exists but that sounds over exaggerating for the moment.
    switch (valign()) {
        case lc::TextConst::VAMiddle:
            alignX += 0.0;
            alignY += -height() / 2. + (height() * .2);
            break;

        case lc::TextConst::VABottom:
            alignX += 0.0;
            alignY += -height() + (height() * .2);
            break;

        case lc::TextConst::VABaseline:
            alignX += 0.0;
            alignY += 0.0;
            break;

        case lc::TextConst::VATop:
            alignX += 0.0;
            alignY += 0.0 + (height() * .2);
            break;

        default:
            break;
    }

    // Horizontal Align:
    switch (halign()) {
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
    painter.translate(insertion_point().x(), -insertion_point().y());
    painter.rotate(-angle());
    painter.translate(alignX, -alignY);
    painter.move_to(0., 0.);
    painter.text(text_value().c_str());
    painter.stroke();
    painter.restore();

    if (modified) {
        painter.restore();
    }

}


