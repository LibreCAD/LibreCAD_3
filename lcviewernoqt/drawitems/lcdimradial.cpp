#include "../lcpainter.h"
#include "../lcdrawoptions.h"
#include "lcdimradial.h"
#include "endcaps.h"
#include <cad/functions/string_helper.h>

LCDimRadial::LCDimRadial(const lc::entity::DimRadial_CSPtr dimRadial) : LCVDrawItem(true), lc::entity::DimRadial(dimRadial, true) {
}

/**
* Draw a DimRadial
* TODO: 1) Draw correct leader
*       2) Change text position such that it's correctly placed under the text. I am not 100% sure how this is done from DXF
*       It could be that Attachmentpoint is simply incorrect?!?!?! or that the DXF importer is incorrect?
*
*/
void LCDimRadial::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    bool modified = false;

    // Decide to show the explecit value or the measured value
    double radiusCircle = this->definitionPoint().distanceTo(this->definitionPoint2());
    const lc::geo::Coordinate& mousePos = middleOfText();
    const bool mouseIsInside = mousePos.distanceTo(definitionPoint()) < radiusCircle;
    // FIXME this should not be fixed
    const double capSize = 2.;
    
    std::string value = lc::StringHelper::dim_value(explicitValue(), options.radialFormat(), radiusCircle);
    
    /* get text size  */
    painter.save();
    painter.font_size(options.dimTextHeight());
    TextExtends te = painter.text_extends(value.c_str());
    painter.restore();
    
    // Draw line
    EndCaps endCaps;
    auto tLinePos = !mouseIsInside
      ? mousePos.move(mousePos - lc::geo::Coordinate(definitionPoint().x(), mousePos.y()), te.width + capSize)
      : mousePos.move(mousePos - lc::geo::Coordinate(definitionPoint().x(), mousePos.y()), -te.width - capSize);

    auto tMText = !mouseIsInside
      ? mousePos.move(mousePos - lc::geo::Coordinate(definitionPoint().x(), mousePos.y()), te.width / 2 + capSize / 2)
      : mousePos.move(mousePos - lc::geo::Coordinate(definitionPoint().x(), mousePos.y()), -te.width / 2 - capSize / 2);
      
    painter.move_to(definitionPoint2().x(), definitionPoint2().y());
    painter.line_to(mousePos.x(), mousePos.y());
    painter.line_to(tLinePos.x(), mousePos.y());
    painter.stroke();
    endCaps.render(painter, EndCaps::CLOSEDARROW, mousePos.x(), mousePos.y(), definitionPoint2().x(), definitionPoint2().y(), capSize) ;

    this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_center, tMText, painter, options, rect);

    if (modified) {
        painter.restore();
    }

}

