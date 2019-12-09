#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "lcdimradial.h"
#include "endcaps.h"
#include <cad/tools/string_helper.h>

using namespace lc::viewer;

LCDimRadial::LCDimRadial(const lc::entity::DimRadial_CSPtr& dimRadial) :
        LCVDrawItem(dimRadial, true),
        _dimRadial(dimRadial) {
}

/**
* Draw a DimRadial
* TODO: 1) Draw correct leader
*       2) Change text position such that it's correctly placed under the text. I am not 100% sure how this is done from DXF
*       It could be that Attachmentpoint is simply incorrect?!?!?! or that the DXF importer is incorrect?
*
*/
void LCDimRadial::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    // Decide to show the explicit value or the measured value
    double radiusCircle = _dimRadial->definitionPoint().distanceTo(_dimRadial->definitionPoint2());
    const lc::geo::Coordinate& mousePos = _dimRadial->middleOfText();
    //const bool mouseIsInside = mousePos.distanceTo(_dimRadial->definitionPoint()) < radiusCircle;
    // FIXME this should not be fixed
    const double capSize = 2.;
    
    std::string value = lc::tools::StringHelper::dim_value(_dimRadial->explicitValue(), options.radialFormat(), radiusCircle);
    
    /* get text size  */
    painter.save();
    painter.font_size(options.dimTextHeight(), false);
    TextExtends te = painter.text_extends(value.c_str());
    painter.restore();
    
    // Draw line
    EndCaps endCaps;

    const bool textisInside = _dimRadial->middleOfText().distanceTo(mousePos) < radiusCircle;
    const bool textFitsInside = radiusCircle * 2. > te.width;

    if (textisInside && textFitsInside) {
        auto tLinePos = (_dimRadial->definitionPoint2().x() > mousePos.x())
                ? lc::geo::Coordinate(mousePos.x() - (te.width + capSize), mousePos.y())
                : lc::geo::Coordinate(mousePos.x() + (te.width + capSize), mousePos.y());
        auto tMText = (_dimRadial->definitionPoint2().x() > mousePos.x())
                ? lc::geo::Coordinate(mousePos.x() - (te.width/2 + capSize/2), mousePos.y() - (te.width/4 + capSize/2))
                : lc::geo::Coordinate(mousePos.x() + (te.width/2 + capSize/2), mousePos.y() - (te.width/4 + capSize/2));

        if (_dimRadial->definitionPoint2().y() <= mousePos.y()) {
            tMText = (_dimRadial->definitionPoint2().x() > mousePos.x())
                    ? lc::geo::Coordinate(mousePos.x() - (te.width/2 + capSize/2), mousePos.y())
                    : lc::geo::Coordinate(mousePos.x() + (te.width/2 + capSize/2), mousePos.y());
        }

        painter.move_to(_dimRadial->definitionPoint2().x(), _dimRadial->definitionPoint2().y());
        painter.line_to(mousePos.x(), mousePos.y());
        painter.line_to(tLinePos.x(), mousePos.y());
        painter.stroke();
        endCaps.render(painter, EndCaps::CLOSEDARROW, mousePos.x(), mousePos.y(), _dimRadial->definitionPoint2().x(), _dimRadial->definitionPoint2().y(), capSize) ;

        drawText(value, _dimRadial->textAngle(), lc::TextConst::AttachmentPoint::Top_center, tMText, painter, options, rect);
    } else {
        auto center = _dimRadial->definitionPoint().mid(_dimRadial->definitionPoint2());
        auto p1 = _dimRadial->definitionPoint2().moveTo(center, -capSize * 1.5);
        auto p2 = _dimRadial->definitionPoint2().moveTo(center, -(te.width + capSize));

        auto tLinePos = (_dimRadial->definitionPoint2().x() < mousePos.x())
                ? lc::geo::Coordinate(p2.x() - (te.width + capSize), p2.y())
                : lc::geo::Coordinate(p2.x() + (te.width + capSize), p2.y());
        lc::geo::Coordinate tMText;

        if (_dimRadial->definitionPoint2().x() < mousePos.x() && _dimRadial->definitionPoint2().y() < mousePos.y()) {
            // checks for 1st quad
            tMText = lc::geo::Coordinate(p2.x() - (te.width/2 + capSize/2), p2.y() - (te.height + capSize));
        } else if (_dimRadial->definitionPoint2().x() >= mousePos.x() && _dimRadial->definitionPoint2().y() < mousePos.y()) {
            // checks for 2nd quad
            tMText = lc::geo::Coordinate(p2.x() + (te.width/2 + capSize/2), p2.y() - (te.height + capSize));
        } else if (_dimRadial->definitionPoint2().x() > mousePos.x() && _dimRadial->definitionPoint2().y() > mousePos.y()) {
            // checks for 3rd quad
            tMText = lc::geo::Coordinate(p2.x() + (te.width/2 + capSize/2), p2.y() + (te.height/4));
        } else if (_dimRadial->definitionPoint2().x() < mousePos.x() && _dimRadial->definitionPoint2().y() >= mousePos.y()) {
            // checks for 4rth quad
            tMText = lc::geo::Coordinate(p2.x() - (te.width/2 + capSize/2), p2.y() + (te.height/4));
        } else {
            // checks for remaining cases
            tMText = lc::geo::Coordinate(p2.x() + (te.width/2 + capSize/2), p2.y() + (te.height/2));
        }

        painter.move_to(_dimRadial->definitionPoint().x(), _dimRadial->definitionPoint().y());
        painter.line_to(_dimRadial->definitionPoint2().x(), _dimRadial->definitionPoint2().y());
        painter.line_to(p2.x(), p2.y());
        painter.line_to(tLinePos.x(), tLinePos.y());
        painter.stroke();
        painter.point(p2.x(), p2.y(), 2, false);
        endCaps.render(painter, EndCaps::CLOSEDARROW, p1.x(), p1.y(), _dimRadial->definitionPoint2().x(), _dimRadial->definitionPoint2().y(), capSize);
        drawText(value, _dimRadial->textAngle(), lc::TextConst::AttachmentPoint::Top_center, tMText, painter, options, rect);
    }
}

lc::entity::CADEntity_CSPtr LCDimRadial::entity() const {
    return _dimRadial;
}

