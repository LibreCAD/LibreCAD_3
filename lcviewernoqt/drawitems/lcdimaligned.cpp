#include "../lcpainter.h"
#include "lcdrawoptions.h"
#include "lcdimaligned.h"
#include "endcaps.h"
#include <cad/functions/str_format.h>

LCDimAligned::LCDimAligned(const lc::DimAligned_CSPtr dimAligned) : LCVDrawItem(true), lc::DimAligned(dimAligned, true) {
}

/**
* Draw a DimRadial
* TODO: 1) When dimension is small then text draw a outside version see https://github.com/LibreCAD/LibreCAD_3/issues/19
*
*/
void LCDimAligned::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {
    bool modified = false;

    // Decide to show the explecit value or the measured value
    std::string value = explicitValue();

    if (value == "<>") {
        value = string_format(options->alignedFormat(), this->definitionPoint3().distanceTo(this->definitionPoint2()));
    } else if (value == "") {
        value = string_format(options->alignedFormat(), this->definitionPoint3().distanceTo(this->definitionPoint2()));
    } else if (value == " ") {
        value = "";
    }

    // Draw line
    EndCaps endCaps;

    lc::geo::Coordinate dir = definitionPoint() - definitionPoint3();
    lc::geo::Coordinate p2 = definitionPoint2() + dir;

    // Draw dimension lines
    painter->move_to(this->definitionPoint3().x(), this->definitionPoint3().y());
    painter->line_to(definitionPoint().x(), definitionPoint().y());
    painter->line_to(p2.x(), p2.y());
    painter->line_to(definitionPoint2().x(), definitionPoint2().y());
    painter->stroke();

    // Draw arrows
    endCaps.render(painter, EndCaps::OPENARROW, definitionPoint().x(), definitionPoint().y(), p2.x(), p2.y(), 10.) ;
    endCaps.render(painter, EndCaps::OPENARROW, p2.x(), p2.y(), this->definitionPoint().x(), this->definitionPoint().y(), 10.) ;


    /** Added to verify the points locations
    painter->move_to(middleOfText().x(), middleOfText().y());
    painter->text("MT");
    painter->move_to(definitionPoint().x(), definitionPoint().y());
    painter->text("0");
    painter->move_to(definitionPoint2().x(), definitionPoint2().y());
    painter->text("2");
    painter->move_to(definitionPoint3().x(), definitionPoint3().y());
    painter->text("3");
    painter->stroke();
    */

    this->drawText(value, this->definitionPoint2().angleTo(this->definitionPoint3()) + textAngle(), this->attachmentPoint(), this->middleOfText(), painter, options, rect);

    if (modified) {
        painter->restore();
    }

}
