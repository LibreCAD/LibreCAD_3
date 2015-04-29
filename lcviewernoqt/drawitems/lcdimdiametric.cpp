#include "../lcpainter.h"
#include "../lcdrawoptions.h"
#include "lcdimdiametric.h"
#include "endcaps.h"
#include <cad/functions/string_helper.h>

LCDimDiametric::LCDimDiametric(const lc::entity::DimDiametric_CSPtr dimDiametric) : LCVDrawItem(true), lc::entity::DimDiametric(dimDiametric, true) {
}

/**
* Draw a DimDiametric
* TODO: draw correct leader and verification if we draw this correctly compared ot other CAD drawings
*/
void LCDimDiametric::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    bool modified = false;

    // Decide to show the explecit value or the measured value
    double diameterCircle = this->definitionPoint().distanceTo(this->definitionPoint2());
    std::string value = lc::StringHelper::dim_value(explicitValue(), options.diametricFormat(), diameterCircle);

    EndCaps endCaps;

    // If a leader needs to get drawn, do so else just take the end point
    // Additionally, if the leader is drawn also make sure the arrow is drawn on the other side
    lc::geo::Coordinate center = this->definitionPoint().mid(this->definitionPoint2());
    double distanceTextToCenter = this->middleOfText().distanceTo(center);

    // Seems like that with radial and diametric there is no choice in attachmentPoint and is 'fixed'
    lc::TextConst::AttachmentPoint aPoint;

    // If the text location is outside of the circle do a full width diameter
    if (distanceTextToCenter >= diameterCircle / 2.) {
        painter.move_to(this->definitionPoint2().x(), this->definitionPoint2().y());
        painter.line_to(this->definitionPoint().x(), this->definitionPoint().y());
        painter.line_to(this->middleOfText().x(), this->middleOfText().y());
        painter.stroke();

        endCaps.render(painter, EndCaps::OPENARROW, this->definitionPoint().x(), this->definitionPoint().y(), this->definitionPoint2().x(), this->definitionPoint2().y(), -10.) ;
        endCaps.render(painter, EndCaps::OPENARROW, this->definitionPoint2().x(), this->definitionPoint2().y(), this->definitionPoint().x(), this->definitionPoint().y(), -10.) ;

        aPoint = lc::TextConst::AttachmentPoint::Top_left;

    } else { // If the text is inside draw a inside
        painter.move_to(this->middleOfText().x(), this->middleOfText().y());
        painter.line_to(this->definitionPoint().x(), this->definitionPoint().y());
        painter.stroke();

        endCaps.render(painter, EndCaps::OPENARROW, this->middleOfText().x(), this->middleOfText().y(), this->definitionPoint().x(), this->definitionPoint().y(), 10.) ;

        aPoint = lc::TextConst::AttachmentPoint::Top_right;
    }

    endCaps.render(painter, EndCaps::CLOSEDROUND, 0., 0., center.x(), center.y(), 2.) ;


    this->drawText(value, this->definitionPoint2().angleTo(this->definitionPoint()) + textAngle(), aPoint, this->middleOfText(), painter, options, rect);

    if (modified) {
        painter.restore();
    }

}

