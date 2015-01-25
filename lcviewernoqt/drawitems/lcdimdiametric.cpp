#include "../lcpainter.h"
#include "lcdrawoptions.h"
#include "lcdimdiametric.h"
#include "endcaps.h"
#include <cad/functions/str_format.h>

LCDimDiametric::LCDimDiametric(const lc::DimDiametric_CSPtr dimDiametric) : LCVDrawItem(true), lc::DimDiametric(dimDiametric, true) {
}

/**
* Draw a DimDiametric
* TODO: draw correct leader and verification if we draw this correctly compared ot other CAD drawings
*/
void LCDimDiametric::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {
    bool modified = false;
    double height = options->dimTextHeight();


    // Decide to show the explecit value or the measured value
    double diameterCircle = this->definitionPoint().distanceTo(this->definitionPoint2());
    std::string value = explicitValue();

    if (value == "<>") {
        value = string_format(options->diametricFormat(), diameterCircle);
    } else if (value == "") {
        value = string_format(options->diametricFormat(), diameterCircle);
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
    }


    EndCaps endCaps;

    // If a leader needs to get drawn, do so else just take the end point
    // Additionally, if the leader is drawn also make sure the arrow is drawn on the other side
    lc::geo::Coordinate center = this->definitionPoint().mid(this->definitionPoint2());
    double distanceTextToCenter = this->middleOfText().distanceTo(center);

    // If the text location is outside of the circle do a full width diameter
    if (distanceTextToCenter >= diameterCircle / 2.) {
        painter->move_to(this->definitionPoint2().x(), this->definitionPoint2().y());
        painter->line_to(this->definitionPoint().x(), this->definitionPoint().y());
        painter->line_to(this->middleOfText().x(), this->middleOfText().y());
        painter->stroke();

        endCaps.render(painter, EndCaps::OPENARROW, this->definitionPoint().x(), this->definitionPoint().y(), this->definitionPoint2().x(), this->definitionPoint2().y(), -10.) ;
        endCaps.render(painter, EndCaps::OPENARROW, this->definitionPoint2().x(), this->definitionPoint2().y(), this->definitionPoint().x(), this->definitionPoint().y(), -10.) ;
    } else { // If the text is inside draw a inside
        painter->move_to(this->middleOfText().x(), this->middleOfText().y());
        painter->line_to(this->definitionPoint().x(), this->definitionPoint().y());
        painter->stroke();

        endCaps.render(painter, EndCaps::OPENARROW, this->middleOfText().x(), this->middleOfText().y(), this->definitionPoint().x(), this->definitionPoint().y(), 10.) ;
    }

    endCaps.render(painter, EndCaps::CLOSEDROUND, 0., 0., center.x(), center.y(), 2.) ;



    // Draw text
    painter->save();
    painter->translate(this->middleOfText().x(), -middleOfText().y());
    painter->rotate(-this->definitionPoint2().angleTo(this->definitionPoint()) + angle());
    painter->translate(alignX, -alignY);
    painter->move_to(0., 0.);
    painter->text(value.c_str());
    painter->stroke();
    painter->restore();


    if (modified) {
        painter->restore();
    }

}

