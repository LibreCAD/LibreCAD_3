#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "lcdimdiametric.h"
#include "endcaps.h"
#include <cad/functions/string_helper.h>

using namespace LCViewer;

LCDimDiametric::LCDimDiametric(const lc::entity::DimDiametric_CSPtr dimDiametric) : 
        LCVDrawItem(dimDiametric, true),
        _dimDiametric(dimDiametric) {
}

/**
* Draw a DimDiametric
* TODO: draw correct leader and verification if we draw this correctly compared ot other CAD drawings
*/
void LCDimDiametric::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {
    // Decide to show the explecit value or the measured value
    auto diameterCircle = _dimDiametric->definitionPoint().distanceTo(_dimDiametric->definitionPoint2());
    auto circle_middle_p0 = _dimDiametric->definitionPoint2().mid(_dimDiametric->definitionPoint());
    // FIXME this should not be fixed
    const double capSize = 2.;

    auto value = lc::StringHelper::dim_value(
            _dimDiametric->explicitValue(),
            options.diametricFormat(),
            diameterCircle
    );

    /* get text size  */
    painter.save();
    painter.font_size(options.dimTextHeight(), false);
    TextExtends te = painter.text_extends(value.c_str());
    painter.restore();

    EndCaps endCaps;
    const bool textisInside = _dimDiametric->middleOfText().distanceTo(circle_middle_p0) < diameterCircle / 2.;
    const bool textFitsInside = diameterCircle > te.width;
    // Draw inside if the text really fit's and if it was initially setup inside
    // No sure if this follow's DXF spec??
    if (textisInside && textFitsInside) {
        painter.move_to(_dimDiametric->definitionPoint().x(), _dimDiametric->definitionPoint().y());
        painter.line_to(_dimDiametric->definitionPoint2().x(), _dimDiametric->definitionPoint2().y());
        painter.stroke();

        endCaps.render(
                painter,
                EndCaps::CLOSEDARROW,
                _dimDiametric->definitionPoint().x(), _dimDiametric->definitionPoint().y(),
                _dimDiametric->definitionPoint2().x(), _dimDiametric->definitionPoint2().y(),
                capSize
        );
        endCaps.render(
                painter,
                EndCaps::CLOSEDARROW,
                _dimDiametric->definitionPoint2().x(), _dimDiametric->definitionPoint2().y(),
                _dimDiametric->definitionPoint().x(), _dimDiametric->definitionPoint().y(),
                capSize
        );

        this->drawText(
                value,
                _dimDiametric->textAngle(),
                lc::TextConst::AttachmentPoint::Top_center,
                _dimDiametric->middleOfText(),
                painter,
                options,
                rect
        );

    }
    else {
        // When we draw text outside, we move the arrows arround (facing inwards) and we draw a small leader where we place the text on
        // here we ignore _dimDiametric->middleOfText() and I am not sure if that's suppose to work like that (RVT)
        auto center = _dimDiametric->definitionPoint().mid(_dimDiametric->definitionPoint2());
        auto p1 = _dimDiametric->definitionPoint().moveTo(center, -capSize * 1.5);
        auto p2 = _dimDiametric->definitionPoint2().moveTo(center, -capSize * 1.5);

        // take largest X value of both _dimDiametric->definition points (on the right) and draw text there
        if (p1.x() > p2.x()) {
            painter.move_to(p1.x() + capSize, p1.y());
            painter.line_to(p1.x(), p1.y());
            painter.line_to(p2.x(), p2.y());
            painter.stroke();
            this->drawText(
                    value,
                    _dimDiametric->textAngle(),
                    lc::TextConst::AttachmentPoint::Top_left,
                    lc::geo::Coordinate(p1.x(), p1.y()),
                    painter,
                    options,
                    rect
            );
        } else {
            painter.move_to(p2.x() + capSize, p2.y());
            painter.line_to(p2.x(), p2.y());
            painter.line_to(p1.x(), p1.y());
            painter.stroke();
            this->drawText(
                    value,
                    _dimDiametric->textAngle(),
                    lc::TextConst::AttachmentPoint::Top_right,
                    lc::geo::Coordinate(p2.x(), p2.y()),
                    painter,
                    options,
                    rect
            );
        }

        endCaps.render(
                painter,
                EndCaps::CLOSEDARROW,
                p1.x(), p1.y(),
                _dimDiametric->definitionPoint().x(), _dimDiametric->definitionPoint().y(),
                capSize
        );
        endCaps.render(
                painter,
                EndCaps::CLOSEDARROW,
                p2.x(), p2.y(),
                _dimDiametric->definitionPoint2().x(), _dimDiametric->definitionPoint2().y(),
                capSize
        );
    }
}

lc::entity::CADEntity_CSPtr LCDimDiametric::entity() const {
    return _dimDiametric;
}

