#include "../lcpainter.h"
#include "lcdrawoptions.h"
#include "lcdimradial.h"
#include "endcaps.h"
#include <cad/functions/str_format.h>

LCDimRadial::LCDimRadial(const lc::DimRadial_CSPtr dimRadial) : LCVDrawItem(true), lc::DimRadial(dimRadial, true) {
}

/**
* Draw a DimRadial
* TODO: 1) Draw correct leader
*       2) Change text position such that it's correctly placed under the text. I am not 100% sure how this is done from DXF
*       It could be that Attachmentpoint is simply incorrect?!?!?! or that the DXF importer is incorrect?
*
*/
void LCDimRadial::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {
    bool modified = false;
    double height = options->dimTextHeight();


    // Decide to show the explecit value or the measured value
    double radiusCircle = this->definitionPoint().distanceTo(this->definitionPoint2());
    std::string value = explicitValue();
    if (value == "<>") {
        value = string_format(options->radialFormat(), radiusCircle);
    } else if (value == " ") {
        value = "";
    }

    painter->font_size(height);
    painter->select_font_face("stick3.ttf");

    TextExtends te = painter->text_extends(value.c_str());
    double alignX = 0.0;
    double alignY = 0.0;

    //    double alignX, alignY;
    // The idea of height * .2 is just a average basline offset. Don't this value to seriously,
    // we could get it from font exists but that sounds over exaggerating for the moment.
    switch (this->attachmentPoint()) {
        case lc::TextConst::AttachmentPoint::Top_left:
            alignY += 0.0 + (height * .2);
            alignX += - te.width;
            break;

        case lc::TextConst::AttachmentPoint::Top_center:
            alignY += 0.0 + (height * .2);
            alignX += - te.width / 2.0;
            break;

        case lc::TextConst::AttachmentPoint::Top_right:
            alignY += 0.0 + (height * .2);
            alignX += 0.;
            break;

        case lc::TextConst::AttachmentPoint::Middle_left:
            alignY += -height / 2. + (height * .2);
            alignX += - te.width;
            break;

        case lc::TextConst::AttachmentPoint::Middle_center:
            alignY += -height / 2. + (height * .2);
            alignX += - te.width / 2.0;
            break;

        case lc::TextConst::AttachmentPoint::Middle_right:
            alignY += -height / 2. + (height * .2);
            alignX += 0.;
            break;

        case lc::TextConst::AttachmentPoint::Bottom_left:
            alignY += -height + (height * .2);
            alignX += - te.width;
            break;

        case lc::TextConst::AttachmentPoint::Bottom_center:
            alignY += -height + (height * .2);
            alignX += - te.width / 2.0;
            break;

        case lc::TextConst::AttachmentPoint::Bottom_right:
            alignY += -height + (height * .2);
            alignX += 0.;
            break;
        default: // Top Center
            alignY += 0.0 + (height * .2);
            alignX += - te.width / 2.0;
            break;
    }


    // Draw line
    EndCaps endCaps;
    double distanceTextToCenter = this->middleOfText().distanceTo(definitionPoint());

    // If a leader needs to get drawn, do so else just take the end point
    // Additionally, if the leader is drawn also make sure the arrow is drawn on the other side
    if (distanceTextToCenter >= radiusCircle) {
        painter->move_to(this->middleOfText().x(), this->middleOfText().y());
        painter->line_to(definitionPoint2().x(), definitionPoint2().y());
        painter->line_to(definitionPoint().x(), definitionPoint().y());
        painter->stroke();
        endCaps.render(painter, EndCaps::OPENARROW, middleOfText().x(), middleOfText().y(), this->definitionPoint2().x(), this->definitionPoint2().y(), 10.) ;
    } else {
        // Draw end caps
        painter->move_to(this->middleOfText().x(), this->middleOfText().y());
        painter->line_to(this->definitionPoint2().x(), this->definitionPoint2().y());
        painter->stroke();
        endCaps.render(painter, EndCaps::OPENARROW, this->middleOfText().x(), this->middleOfText().y(), this->definitionPoint2().x(), this->definitionPoint2().y(), 10.) ;
    }

    endCaps.render(painter, EndCaps::CLOSEDROUND, 0., 0., this->definitionPoint().x(), this->definitionPoint().y(), 2.) ;


    // Draw text
    painter->save();
    painter->translate(this->middleOfText().x(), -middleOfText().y());
    painter->rotate(this->definitionPoint().angleTo(this->definitionPoint2()) + angle());
    painter->translate(alignX, -alignY);
    painter->move_to(0., 0.);
    painter->text(value.c_str());
    painter->stroke();
    painter->restore();


    if (modified) {
        painter->restore();
    }

}

