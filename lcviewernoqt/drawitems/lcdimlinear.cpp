#include "../lcpainter.h"
#include "lcdrawoptions.h"
#include "lcdimlinear.h"
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
    bool isHorizontal = false;
    const double capSize = 10.;

    const lc::geo::Coordinate &firstPoint = (definitionPoint2().magnitude() <= definitionPoint3().magnitude())
                                          ? definitionPoint2() : definitionPoint3();
                                          
    const lc::geo::Coordinate &secondPoint = (definitionPoint2().magnitude() <= definitionPoint3().magnitude())
                                           ? definitionPoint3() : definitionPoint2();
                                           
    const lc::geo::Coordinate& mousePos = middleOfText();

    /* TODO maybe need to check for vertical too ?
     * asume firstPoint is always on left side...
     */
    if (!(mousePos.x() <= firstPoint.x() || mousePos.x() >= secondPoint.x())
        && !(mousePos.y() >= firstPoint.y() && mousePos.y() <= secondPoint.y())) {
        isHorizontal = true;
    }

    // Decide to show the explecit value or the measured value
    std::string value = explicitValue();

    if (value == "<>") {
        value = string_format(options->linearFormat(), isHorizontal ? std::abs(dx) : std::abs(dy));
    } else if (value == "") {
        value = string_format(options->linearFormat(), isHorizontal ? std::abs(dx) : std::abs(dy));
    } else if (value == " ") {
        value = "";
    }

    // get text size
    painter->save();
    painter->font_size(options->dimTextHeight());
    TextExtends te = painter->text_extends(value.c_str());
    painter->restore();

    // Draw line
    EndCaps endCaps;
    const bool shortOnSpace = ((isHorizontal ? std::abs(dx) : std::abs(dy)) - te.width - capSize * 2) < 0;

    /* If a leader needs to get drawn, do so else just take the end point
     * Additionally, if the leader is drawn also make sure the arrow is drawn on the other side
     * TODO  side alignment */
    if (isHorizontal && shortOnSpace) {

        // right
        painter->move_to(secondPoint.x(), secondPoint.y());
        painter->line_to(secondPoint.x(), mousePos.y() + (mousePos.y() >= secondPoint.y() ? capSize / 2 : -capSize / 2));
        painter->move_to(secondPoint.x(), mousePos.y());

        // middle line TODO which side text would be
        painter->line_to(secondPoint.x() + capSize + capSize * 0.5 + te.width, mousePos.y());
        painter->move_to(firstPoint.x(), mousePos.y());
        painter->line_to(firstPoint.x() - capSize * 2, mousePos.y());

        // left
        painter->move_to(firstPoint.x(), mousePos.y() + (mousePos.y() >= firstPoint.y() ? capSize / 2 : -capSize / 2));
        painter->line_to(firstPoint.x(), firstPoint.y());
        painter->stroke();

        endCaps.render(painter, EndCaps::CLOSEDARROW, secondPoint.x() + capSize * 2, mousePos.y(), secondPoint.x(), mousePos.y(), capSize) ;
        endCaps.render(painter, EndCaps::CLOSEDARROW, firstPoint.x() - capSize * 2, mousePos.y(), firstPoint.x(), mousePos.y(), capSize) ;

    } else if (isHorizontal) {

        // right
        painter->move_to(secondPoint.x(), secondPoint.y());
        painter->line_to(secondPoint.x(), mousePos.y() + (mousePos.y() >= secondPoint.y() ? capSize / 2 : -capSize / 2));
        painter->move_to(secondPoint.x(), mousePos.y());

        // middle line
        painter->line_to(firstPoint.x(), mousePos.y());

        // left
        painter->move_to(firstPoint.x(), mousePos.y() + (mousePos.y() >= firstPoint.y() ? capSize / 2 : -capSize / 2));
        painter->line_to(firstPoint.x(), firstPoint.y());
        painter->stroke();

        endCaps.render(painter, EndCaps::CLOSEDARROW, secondPoint.x(), mousePos.y(), firstPoint.x(), mousePos.y(), capSize) ;
        endCaps.render(painter, EndCaps::CLOSEDARROW, firstPoint.x(), mousePos.y(), secondPoint.x(), mousePos.y(), capSize) ;

    } else if (!isHorizontal && shortOnSpace) {

        // top
        painter->move_to(secondPoint.x(), secondPoint.y());
        painter->line_to(mousePos.x() + (mousePos.x() >= secondPoint.x() ? capSize / 2 : -capSize / 2), secondPoint.y());
        painter->move_to(mousePos.x(), secondPoint.y());

        // middle line
        painter->line_to(mousePos.x(), secondPoint.y() + capSize * 2);
        painter->move_to(mousePos.x(), firstPoint.y());
        painter->line_to(mousePos.x(), firstPoint.y() - capSize * 2);

        // bottom
        painter->move_to(mousePos.x() + (mousePos.x() >= firstPoint.x() ? capSize / 2 : -capSize / 2), firstPoint.y());
        painter->line_to(firstPoint.x(), firstPoint.y());
        painter->stroke();

        endCaps.render(painter, EndCaps::CLOSEDARROW, mousePos.x(), secondPoint.y() + capSize * 2, mousePos.x(), secondPoint.y(), capSize) ;
        endCaps.render(painter, EndCaps::CLOSEDARROW, mousePos.x(), firstPoint.y() - capSize * 2, mousePos.x(), firstPoint.y(), capSize) ;


    } else if (!isHorizontal) {

        // top
        painter->move_to(secondPoint.x(), secondPoint.y());
        painter->line_to(mousePos.x() + (mousePos.x() >= secondPoint.x() ? capSize / 2 : -capSize / 2), secondPoint.y());
        painter->move_to(mousePos.x(), secondPoint.y());
        //
        // middle line
        painter->line_to(mousePos.x(), firstPoint.y());

        // bottom
        painter->move_to(mousePos.x() + (mousePos.x() >= firstPoint.x() ? capSize / 2 : -capSize / 2), firstPoint.y());
        painter->line_to(firstPoint.x(), firstPoint.y());
        painter->stroke();

        endCaps.render(painter, EndCaps::CLOSEDARROW, mousePos.x(), secondPoint.y(), mousePos.x(), firstPoint.y(), capSize) ;
        endCaps.render(painter, EndCaps::CLOSEDARROW, mousePos.x(), firstPoint.y(), mousePos.x(), secondPoint.y(), capSize) ;
    }

    /* Added to verify the points locations
    painter->move_to(rect.width(), middleOfText().y());
    painter->text("MT");

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



    if (isHorizontal && shortOnSpace) {

        // TODO on short left or right side ?
        this->drawText(value, (isHorizontal ? 0. : (90. / 180.) * M_PI) + textAngle(), attachmentPoint(), {mousePos.x() + capSize * 2.5, mousePos.y()}, painter, options, rect);

    } else if (isHorizontal) {

        this->drawText(value, (isHorizontal ? 0. : (90. / 180.) * M_PI) + textAngle(), attachmentPoint(), middleOfText(), painter, options, rect);

    } else if (!isHorizontal && shortOnSpace) {

        // vertical, hard coded top line
        painter->move_to(mousePos.x(), secondPoint.y() + capSize * 2);
        painter->line_to(mousePos.x() + te.width, secondPoint.y() + capSize * 2);
        painter->stroke();
        this->drawText(value, 0., lc::TextConst::AttachmentPoint::Top_right, {mousePos.x(), secondPoint.y() + capSize * 2}, painter, options, rect);

    } else {
        // vertical, text rotated based on side
        const double rotationAngle = mousePos.x() >= secondPoint.x() ? -90. / 180. : 90. / 180.;

        this->drawText(value, (rotationAngle * M_PI) + textAngle(), attachmentPoint(), middleOfText(), painter, options, rect);

    }

    if (modified) {
        painter->restore();
    }

}
