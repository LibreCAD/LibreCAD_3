#include "../lcpainter.h"
#include "../lcdrawoptions.h"
#include "lcdimdiametric.h"
#include "endcaps.h"
#include <cad/functions/string_helper.h>

using namespace LCViewer;

LCDimDiametric::LCDimDiametric(const lc::entity::DimDiametric_CSPtr dimDiametric) : LCVDrawItem(true), lc::entity::DimDiametric(dimDiametric, true) {
}

/**
* Draw a DimDiametric
* TODO: draw correct leader and verification if we draw this correctly compared ot other CAD drawings
*/
void LCDimDiametric::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {
    bool modified = false;
    // Decide to show the explecit value or the measured value
    auto diameterCircle = this->definitionPoint().distanceTo(this->definitionPoint2());
    auto circle_middle_p0 = definitionPoint2().mid(definitionPoint());
    // FIXME this should not be fixed
    const double capSize = 2.;

    auto value = lc::StringHelper::dim_value(explicitValue(), options.diametricFormat(), diameterCircle);

    /* get text size  */
    painter.save();
    painter.font_size(options.dimTextHeight());
    TextExtends te = painter.text_extends(value.c_str());
    painter.restore();

    EndCaps endCaps;
    const bool textisInside = middleOfText().distanceTo(circle_middle_p0) < diameterCircle / 2.;
    const bool textFitsInside = diameterCircle > te.width;
    // Draw inside if the text really fit's and if it was initially setup inside
    // No sure if this follow's DXF spec??
    if (textisInside && textFitsInside) {
        painter.move_to(definitionPoint().x(), definitionPoint().y());
        painter.line_to(definitionPoint2().x(), definitionPoint2().y());
        painter.stroke();
        endCaps.render(painter, EndCaps::CLOSEDARROW, definitionPoint().x(), definitionPoint().y(), definitionPoint2().x(), definitionPoint2().y(), capSize);
        endCaps.render(painter, EndCaps::CLOSEDARROW, definitionPoint2().x(), definitionPoint2().y(), definitionPoint().x(), definitionPoint().y(), capSize);
        this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_center, middleOfText(), painter, options, rect);

    } else {
        // When we draw text outside, we move the arrows arround (facing inwards) and we draw a small leader where we place the text on
        // here we ignore middleOfText() and I am not sure if that's suppose to work like that (RVT)
        auto center = definitionPoint().mid(definitionPoint2());
        auto p1 = definitionPoint().moveTo(center, -capSize * 1.5);
        auto p2 = definitionPoint2().moveTo(center, -capSize * 1.5);

        // take largest X value of both definition points (on the right) and draw text there
        if (p1.x() > p2.x()) {
            painter.move_to(p1.x() + capSize, p1.y());
            painter.line_to(p1.x(), p1.y());
            painter.line_to(p2.x(), p2.y());
            painter.stroke();
            this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_left, lc::geo::Coordinate(p1.x(), p1.y()), painter, options, rect);
        } else {
            painter.move_to(p2.x() + capSize, p2.y());
            painter.line_to(p2.x(), p2.y());
            painter.line_to(p1.x(), p1.y());
            painter.stroke();
            this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_right, lc::geo::Coordinate(p2.x(), p2.y()), painter, options, rect);
        }
        endCaps.render(painter, EndCaps::CLOSEDARROW, p1.x(), p1.y(), definitionPoint().x(), definitionPoint().y(), capSize);
        endCaps.render(painter, EndCaps::CLOSEDARROW, p2.x(), p2.y(), definitionPoint2().x(), definitionPoint2().y(), capSize);
    }

    if (modified) {
        painter.restore();
    }

}

/*
void LCDimDiametric::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {
    bool modified = false;

    // Decide to show the explecit value or the measured value
    double diameterCircle = definitionPoint().distanceTo(this->definitionPoint2());

    // FIXME this should not be fixed
    const double capSize = 2.;

    get text size
    std::string value = lc::StringHelper::dim_value(explicitValue(), options.diametricFormat(), diameterCircle);
    painter.save();
    painter.font_size(options.dimTextHeight());
    TextExtends te = painter.text_extends(value.c_str());
    painter.restore();



    // Draw line
    EndCaps endCaps;

    const bool textFitsInside = diameterCircle > te.width;
    auto tMText = this->middleOfText();
    if (textFitsInside) {
        painter.move_to(definitionPoint().x(),definitionPoint().y());
        painter.line_to(definitionPoint2().x(),definitionPoint2().y());
        painter.stroke();
        endCaps.render(painter, EndCaps::CLOSEDARROW, definitionPoint().x(),definitionPoint().y(), definitionPoint2().x(),definitionPoint2().y(), capSize) ;
        endCaps.render(painter, EndCaps::CLOSEDARROW,  definitionPoint2().x(),definitionPoint2().y(), definitionPoint().x(),definitionPoint().y(),capSize) ;
        this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_center, tMText, painter, options, rect);

    } else {
        auto center = definitionPoint().mid(definitionPoint2());
        auto p1 = definitionPoint().moveTo(center, -capSize*1.5);
        auto p2 = definitionPoint2().moveTo(center, -capSize*1.5);

        // take largest X value of both definition points (on the right) and draw text there
        if (p1.x()>p2.x()) {
            painter.move_to(p1.x()+capSize, p1.y());
            painter.line_to(p1.x(), p1.y());
            painter.line_to(p2.x(), p2.y());
            painter.stroke();
            this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_left, lc::geo::Coordinate(p1.x(), p1.y()), painter, options, rect);
        } else {
            painter.move_to(p2.x()+capSize, p2.y());
            painter.line_to(p2.x(), p2.y());
            painter.line_to(p1.x(), p1.y());
            painter.stroke();
            this->drawText(value, textAngle(), lc::TextConst::AttachmentPoint::Top_right, lc::geo::Coordinate(p2.x(), p2.y()), painter, options, rect);
        }
        endCaps.render(painter, EndCaps::CLOSEDARROW, p1.x(),p1.y(),definitionPoint().x(),definitionPoint().y(), capSize) ;
        endCaps.render(painter, EndCaps::CLOSEDARROW, p2.x(),p2.y(), definitionPoint2().x(),definitionPoint2().y(), capSize) ;
    }

    if (modified) {
        painter.restore();
    }

}
 */


