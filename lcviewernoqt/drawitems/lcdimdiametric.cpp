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
void LCDimDiametric::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {
    bool modified = false;

    // Decide to show the explecit value or the measured value
    double diameterCircle = this->definitionPoint().distanceTo(this->definitionPoint2());
    const auto circle_middle_p0 = definitionPoint2() / 2;
    const auto &circle_p2 = definitionPoint();
    const lc::geo::Coordinate &mousePos = middleOfText();
    const bool mouseIsInside = mousePos.distanceTo(circle_middle_p0) < circle_middle_p0.distanceTo(circle_p2);
    // FIXME this should not be fixed
    const double capSize = 2.;

    std::string value = lc::StringHelper::dim_value(explicitValue(), options.diametricFormat(), diameterCircle);


    /* get text size  */
    painter.save();
    painter.font_size(options.dimTextHeight());
    TextExtends te = painter.text_extends(value.c_str());
    painter.restore();

    // Draw line
    EndCaps endCaps;
    auto tLinePos = !mouseIsInside
                    ? mousePos.move(mousePos - lc::geo::Coordinate(circle_middle_p0.x(), mousePos.y()), te.width + capSize)
                    : mousePos.move(mousePos - lc::geo::Coordinate(circle_middle_p0.x(), mousePos.y()), -te.width - capSize);

    auto tMText = !mouseIsInside
                  ? mousePos.move(mousePos - lc::geo::Coordinate(circle_middle_p0.x(), mousePos.y()), te.width / 2 + capSize / 2)
                  : mousePos.move(mousePos - lc::geo::Coordinate(circle_middle_p0.x(), mousePos.y()), -te.width / 2 - capSize / 2);

    painter.move_to(circle_p2.x(), circle_p2.y());
    painter.line_to(mousePos.x(), mousePos.y());
    painter.line_to(tLinePos.x(), mousePos.y());
    painter.stroke();
    endCaps.render(painter, EndCaps::CLOSEDARROW, mousePos.x(), mousePos.y(), circle_p2.x(), circle_p2.y(), capSize) ;

    this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_center, tMText, painter, options, rect);

    if (modified) {
        painter.restore();
    }

}


