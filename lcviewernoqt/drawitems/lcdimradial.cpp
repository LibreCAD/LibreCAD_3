#include "../lcpainter.h"
#include "../lcdrawoptions.h"
#include "lcdimradial.h"
#include "endcaps.h"
#include <cad/functions/string_helper.h>
using namespace LCViewer;
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
    //const bool mouseIsInside = mousePos.distanceTo(definitionPoint()) < radiusCircle;
    // FIXME this should not be fixed
    const double capSize = 2.;
    
    std::string value = lc::StringHelper::dim_value(explicitValue(), options.radialFormat(), radiusCircle);
    
    /* get text size  */
    painter.save();
    painter.font_size(options.dimTextHeight(), false);
    TextExtends te = painter.text_extends(value.c_str());
    painter.restore();
    
    // Draw line
    EndCaps endCaps;

    const bool textisInside = middleOfText().distanceTo(mousePos) < radiusCircle;
    const bool textFitsInside = radiusCircle * 2. > te.width;

    if (textisInside && textFitsInside) {
        auto tLinePos = (definitionPoint2().x() > mousePos.x())
                ? lc::geo::Coordinate(mousePos.x() - (te.width + capSize), mousePos.y())
                : lc::geo::Coordinate(mousePos.x() + (te.width + capSize), mousePos.y());
        auto tMText = (definitionPoint2().x() > mousePos.x())
                ? lc::geo::Coordinate(mousePos.x() - (te.width/2 + capSize/2), mousePos.y() - (te.width/4 + capSize/2))
                : lc::geo::Coordinate(mousePos.x() + (te.width/2 + capSize/2), mousePos.y() - (te.width/4 + capSize/2));

        if (definitionPoint2().y() <= mousePos.y()) {
            tMText = (definitionPoint2().x() > mousePos.x())
                    ? lc::geo::Coordinate(mousePos.x() - (te.width/2 + capSize/2), mousePos.y())
                    : lc::geo::Coordinate(mousePos.x() + (te.width/2 + capSize/2), mousePos.y());
        }

        painter.move_to(definitionPoint2().x(), definitionPoint2().y());
        painter.line_to(mousePos.x(), mousePos.y());
        painter.line_to(tLinePos.x(), mousePos.y());
        painter.stroke();
        endCaps.render(painter, EndCaps::CLOSEDARROW, mousePos.x(), mousePos.y(), definitionPoint2().x(), definitionPoint2().y(), capSize) ;

        this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_center, tMText, painter, options, rect);
    } else {
        auto center = definitionPoint().mid(definitionPoint2());
        auto p1 = definitionPoint2().moveTo(center, -capSize * 1.5);
        auto p2 = definitionPoint2().moveTo(center, -(te.width + capSize));

        auto tLinePos = (definitionPoint2().x() < mousePos.x())
                ? lc::geo::Coordinate(p2.x() - (te.width + capSize), p2.y())
                : lc::geo::Coordinate(p2.x() + (te.width + capSize), p2.y());
        lc::geo::Coordinate tMText;

        if (definitionPoint2().x() < mousePos.x() && definitionPoint2().y() < mousePos.y()) {
            // checks for 1st quad
            tMText = lc::geo::Coordinate(p2.x() - (te.width/2 + capSize/2), p2.y() - (te.height + capSize));
        } else if (definitionPoint2().x() >= mousePos.x() && definitionPoint2().y() < mousePos.y()) {
            // checks for 2nd quad
            tMText = lc::geo::Coordinate(p2.x() + (te.width/2 + capSize/2), p2.y() - (te.height + capSize));
        } else if (definitionPoint2().x() > mousePos.x() && definitionPoint2().y() > mousePos.y()) {
            // checks for 3rd quad
            tMText = lc::geo::Coordinate(p2.x() + (te.width/2 + capSize/2), p2.y() + (te.height/4));
        } else if (definitionPoint2().x() < mousePos.x() && definitionPoint2().y() >= mousePos.y()) {
            // checks for 4rth quad
            tMText = lc::geo::Coordinate(p2.x() - (te.width/2 + capSize/2), p2.y() + (te.height/4));
        } else {
            // checks for remaining cases
            tMText = lc::geo::Coordinate(p2.x() + (te.width/2 + capSize/2), p2.y() + (te.height/2));
        }

        painter.move_to(definitionPoint().x(), definitionPoint().y());
        painter.line_to(definitionPoint2().x(), definitionPoint2().y());
        painter.line_to(p2.x(), p2.y());
        painter.line_to(tLinePos.x(), tLinePos.y());
        painter.stroke();
        painter.point(p2.x(), p2.y(), 2, false);
        endCaps.render(painter, EndCaps::CLOSEDARROW, p1.x(), p1.y(), definitionPoint2().x(), definitionPoint2().y(), capSize);
        this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_center, tMText, painter, options, rect);
    }

    if (modified) {
        painter.restore();
    }

}

