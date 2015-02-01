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

    // Decide to show the explecit value or the measured value
    double radiusCircle = this->definitionPoint().distanceTo(this->definitionPoint2());
    std::string value = explicitValue();

    if (value == "<>") {
        value = string_format(options->radialFormat(), radiusCircle);
    } else if (value == "") {
        value = string_format(options->radialFormat(), radiusCircle);
    } else if (value == " ") {
        value = "";
    }

    // Draw line
    EndCaps endCaps;
    double distanceTextToCenter = this->middleOfText().distanceTo(definitionPoint());

    // Seems like that with radial and diametric there is no choice in attachmentPoint and is 'fixed'
    lc::TextConst::AttachmentPoint aPoint;

    // If a leader needs to get drawn, do so else just take the end point
    // Additionally, if the leader is drawn also make sure the arrow is drawn on the other side
    if (distanceTextToCenter >= radiusCircle) {
        painter->move_to(this->middleOfText().x(), this->middleOfText().y());
        painter->line_to(definitionPoint2().x(), definitionPoint2().y());
        painter->line_to(definitionPoint().x(), definitionPoint().y());
        painter->stroke();
        endCaps.render(painter, EndCaps::OPENARROW, middleOfText().x(), middleOfText().y(), this->definitionPoint2().x(), this->definitionPoint2().y(), 10.) ;

        aPoint = lc::TextConst::AttachmentPoint::Top_left;
    } else {
        // Draw end caps
        painter->move_to(this->middleOfText().x(), this->middleOfText().y());
        painter->line_to(this->definitionPoint2().x(), this->definitionPoint2().y());
        painter->stroke();
        endCaps.render(painter, EndCaps::OPENARROW, this->middleOfText().x(), this->middleOfText().y(), this->definitionPoint2().x(), this->definitionPoint2().y(), 10.) ;
        aPoint = lc::TextConst::AttachmentPoint::Top_right;
    }

    endCaps.render(painter, EndCaps::CLOSEDROUND, 0., 0., this->definitionPoint().x(), this->definitionPoint().y(), 2.) ;

    this->drawText(value, this->definitionPoint().angleTo(this->definitionPoint2()) + textAngle(), aPoint, this->middleOfText(), painter, options, rect);


    if (modified) {
        painter->restore();
    }

}

