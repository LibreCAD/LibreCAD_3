#include "../lcpainter.h"
#include "lcdrawoptions.h"
#include "lcdimaligned.h"
#include "endcaps.h"
#include <cad/functions/string_helper.h>

LCDimAligned::LCDimAligned(const lc::DimAligned_CSPtr dimAligned) : LCVDrawItem(true), lc::DimAligned(dimAligned, true) {
}

/**
* Draw a DimRadial
* TODO: 1) When dimension is small then text draw a outside version see https://github.com/LibreCAD/LibreCAD_3/issues/19
*
*/
void LCDimAligned::draw(LcPainter *painter, LcDrawOptions *options, const lc::geo::Area &rect) const {
    bool modified = false;
    const double capSize = 10.;
    //const lc::geo::Coordinate &mousePos = middleOfText();

    // Decide to show the explecit value or the measured value
    std::string value = lc::string_helper::dim_value(explicitValue(), options->alignedFormat(), this->definitionPoint3().distanceTo(this->definitionPoint2()));

    // get text size
    painter->save();
    painter->font_size(options->dimTextHeight());
    TextExtends te = painter->text_extends(value.c_str());
    painter->restore();

    // Draw line
    EndCaps endCaps;
    bool shortOnSpace = (definitionPoint3().distanceTo(definitionPoint2()) - te.width - capSize * 2) < 0;

    lc::geo::Coordinate p2 = definitionPoint2() + (definitionPoint() - definitionPoint3());

    // Draw dimension lines
    if (shortOnSpace) {
        painter->move_to(definitionPoint3().x(), definitionPoint3().y());
        painter->line_to(definitionPoint().x(), definitionPoint().y());

        auto d0ext = definitionPoint().move(definitionPoint() - p2, capSize * 2);
        painter->move_to(d0ext.x(), d0ext.y());
        painter->line_to(definitionPoint().x(), definitionPoint().y());
        
        auto p2ext = p2.move(definitionPoint() - p2, -(capSize * 2));
        painter->move_to(p2ext.x(), p2ext.y());
        painter->line_to(p2.x(), p2.y());
        painter->line_to(definitionPoint2().x(), definitionPoint2().y());
        
        
        /* draw a nice line for text
         * TODO maybe we will need mousePos for this in future
         */
        if (std::abs(d0ext.angleTo(p2ext)) >= 90. / 180.*M_PI) {
            painter->move_to(d0ext.x() + te.width, d0ext.y());
            painter->line_to(d0ext.x(), d0ext.y());
            this->drawText(value, textAngle(), this->attachmentPoint(), {d0ext.x() + te.width / 2, d0ext.y()}, painter, options, rect);
        } else {
            painter->move_to(p2ext.x() + te.width, p2ext.y());
            painter->line_to(p2ext.x(), p2ext.y());
            this->drawText(value, textAngle(), this->attachmentPoint(), {p2ext.x() + te.width / 2, p2ext.y()}, painter, options, rect);
        }

        painter->stroke();

        // Draw arrows

        endCaps.render(painter, EndCaps::CLOSEDARROW, d0ext.x(), d0ext.y(), definitionPoint().x(), definitionPoint().y(), capSize) ;
        endCaps.render(painter, EndCaps::CLOSEDARROW, p2ext.x(), p2ext.y(), p2.x(), p2.y(), capSize) ;

    } else {

        painter->move_to(this->definitionPoint3().x(), this->definitionPoint3().y());
        painter->line_to(definitionPoint().x(), definitionPoint().y());
        painter->line_to(p2.x(), p2.y());
        painter->line_to(definitionPoint2().x(), definitionPoint2().y());
        painter->stroke();

        // Draw arrows
        endCaps.render(painter, EndCaps::CLOSEDARROW, definitionPoint().x(), definitionPoint().y(), p2.x(), p2.y(), capSize) ;
        endCaps.render(painter, EndCaps::CLOSEDARROW, p2.x(), p2.y(), this->definitionPoint().x(), this->definitionPoint().y(), capSize) ;
        this->drawText(value, this->definitionPoint2().angleTo(this->definitionPoint3()) + textAngle(), this->attachmentPoint(), this->middleOfText(), painter, options, rect);
    }

    /* Added to verify the points locations
    painter->move_to(middleOfText().x(), middleOfText().y());
    painter->text("MT");
    painter->move_to(definitionPoint().x(), definitionPoint().y());
    painter->text("0");
    painter->move_to(definitionPoint2().x(), definitionPoint2().y());
    painter->text("2");
    painter->move_to(definitionPoint3().x(), definitionPoint3().y());
    painter->text("3");
    painter->move_to(p2.x(), p2.y());
    painter->text("p2");
    painter->stroke();*/

    if (modified) {
        painter->restore();
    }

}
