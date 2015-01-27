#include "../lcpainter.h"
#include "lcdrawoptions.h"
#include "LCDimLinear.h"
#include "endcaps.h"
#include <cad/functions/str_format.h>

LCDimLinear::LCDimLinear(const lc::DimLinear_CSPtr dimLinear) : LCVDrawItem(true), lc::DimLinear(dimLinear, true) {
}

/**
* Draw a DimRadial
* TODO: 1) Draw linear dimension with correct edges/dots
*       2) When oblique is set, verify if we calculate length correctly. Not sure if that's invluenced
*       3) Draw oblique correctly see http://www.cad-notes.com/autocad-isometric-text-and-dimension/ can possebly be drawn using painter's skew (see transformations)
*
*/
void LCDimLinear::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {
    bool modified = false;

    const double dx = this->definitionPoint3().x() - this->definitionPoint2().x();
    const double dy = this->definitionPoint3().y() - this->definitionPoint2().y();
    const bool isHorizontal = std::abs(definitionPoint().x() - definitionPoint3().x()) < LCTOLERANCE;

    // Decide to show the explecit value or the measured value
    std::string value = explicitValue();

    if (value == "<>") {
        value = string_format(options->linearFormat(), isHorizontal ? std::abs(dx) : std::abs(dy));
    } else if (value == "") {
        value = string_format(options->linearFormat(), isHorizontal ? std::abs(dx) : std::abs(dy));
    } else if (value == " ") {
        value = "";
    }

    // Draw line
    EndCaps endCaps;

    // If a leader needs to get drawn, do so else just take the end point
    // Additionally, if the leader is drawn also make sure the arrow is drawn on the other side
    if (isHorizontal) {
        painter->move_to(this->definitionPoint3().x(), this->definitionPoint3().y());
        painter->line_to(definitionPoint().x(), definitionPoint().y());
        painter->line_to(definitionPoint2().x(), definitionPoint().y());
        painter->line_to(definitionPoint2().x(), definitionPoint2().y());
        painter->stroke();

        endCaps.render(painter, EndCaps::OPENARROW, definitionPoint().x(), definitionPoint().y(), this->definitionPoint2().x(), this->definitionPoint().y(), 10.) ;
        endCaps.render(painter, EndCaps::OPENARROW, definitionPoint2().x(), definitionPoint().y(), this->definitionPoint().x(), this->definitionPoint().y(), 10.) ;
    } else {
        painter->move_to(this->definitionPoint3().x(), this->definitionPoint3().y());
        painter->line_to(definitionPoint().x(), definitionPoint().y());
        painter->line_to(definitionPoint().x(), definitionPoint2().y());
        painter->line_to(definitionPoint2().x(), definitionPoint2().y());
        painter->stroke();

        endCaps.render(painter, EndCaps::OPENARROW, definitionPoint().x(), definitionPoint().y(), this->definitionPoint().x(), this->definitionPoint2().y(), 10.) ;
        endCaps.render(painter, EndCaps::OPENARROW, definitionPoint().x(), definitionPoint2().y(), this->definitionPoint().x(), this->definitionPoint().y(), 10.) ;
    }

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

    this->drawText(value,(isHorizontal ? 0. : (90. / 180.) * M_PI) + textAngle(), this->attachmentPoint(), this->middleOfText(), painter, options, rect);


    if (modified) {
        painter->restore();
    }

}
