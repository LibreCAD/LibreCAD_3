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
    painter.font_size(options.dimTextHeight());
    TextExtends te = painter.text_extends(value.c_str());
    painter.restore();
    
    // Draw line
    EndCaps endCaps;
    auto tLinePos = lc::geo::Coordinate(0.0,0.0,0.0);
    auto tMText = lc::geo::Coordinate(0.0,0.0,0.0);
    auto p1 = lc::geo::Coordinate(0.0,0.0,0.0);
    auto p2 = lc::geo::Coordinate(0.0,0.0,0.0);

    const bool textisInside = middleOfText().distanceTo(mousePos) < radiusCircle;
    const bool textFitsInside = radiusCircle * 2. > te.width;

    if (textisInside && textFitsInside) {
        tLinePos = (definitionPoint2().x() > mousePos.x())
                ? lc::geo::Coordinate(mousePos.x() - (te.width + capSize), mousePos.y())
                : lc::geo::Coordinate(mousePos.x() + (te.width + capSize), mousePos.y());

        if (definitionPoint2().y() > mousePos.y()) {
            tMText = (definitionPoint2().x() > mousePos.x())
                    ? lc::geo::Coordinate(mousePos.x() - (te.width/2 + capSize/2), mousePos.y() - (te.width/4 + capSize/2))
                    : lc::geo::Coordinate(mousePos.x() + (te.width/2 + capSize/2), mousePos.y() - (te.width/4 + capSize/2));
        } else {
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
        p1 = definitionPoint2().moveTo(center, -capSize * 1.5);
        p2 = definitionPoint2().moveTo(center, -(te.width + capSize));

        tLinePos = (definitionPoint2().x() < mousePos.x())
                ? lc::geo::Coordinate(p2.x() - (te.width + capSize), p2.y())
                : lc::geo::Coordinate(p2.x() + (te.width + capSize), p2.y());


        if (definitionPoint2().y() > mousePos.y()) {
            tMText = (definitionPoint2().x() > mousePos.x())
                    ? lc::geo::Coordinate(p2.x() + (te.width/2 + capSize/2), p2.y() + (te.height/4))
                    : lc::geo::Coordinate(p2.x() - (te.width/2 + capSize/2), p2.y() + (te.height/4));
        } else if (definitionPoint2().y() == mousePos.y()){
            tLinePos = p2; // No need for extended line
            tMText = (definitionPoint2().x() > mousePos.x())
                    ? lc::geo::Coordinate(p2.x() - (te.width/4 + capSize/4), p2.y() + (te.height/2))
                    : lc::geo::Coordinate(p2.x() + (te.width/4 + capSize/4), p2.y() + (te.height/2));
        } else {
            tMText = (definitionPoint2().x() > mousePos.x())
                    ? lc::geo::Coordinate(p2.x() + (te.width/2 + capSize/2), p2.y() - (te.height + capSize))
                    : lc::geo::Coordinate(p2.x() - (te.width/2 + capSize/2), p2.y() - (te.height + capSize));
        }

        if (definitionPoint2().x() == mousePos.x())
        {
            tMText = (definitionPoint2().y() > mousePos.y())
                    ? lc::geo::Coordinate(p2.x() + (te.width/2 + capSize/2), p2.y() + (te.height/2))
                    : lc::geo::Coordinate(p2.x() + (te.width/2 + capSize/2), p2.y() - (te.height + capSize));
        }


        painter.move_to(definitionPoint().x(), definitionPoint().y());
        painter.line_to(definitionPoint2().x(), definitionPoint2().y());
        painter.line_to(p2.x(), p2.y());
        painter.line_to(tLinePos.x(), tLinePos.y());
        painter.stroke();
        painter.point(p2.x(), p2.y(), 2, false);
        endCaps.render(painter, EndCaps::CLOSEDARROW, p1.x(), p1.y(), definitionPoint2().x(), definitionPoint2().y(), capSize);
    }

    this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_center, tMText, painter, options, rect);

    if (modified) {
        painter.restore();
    }

}

