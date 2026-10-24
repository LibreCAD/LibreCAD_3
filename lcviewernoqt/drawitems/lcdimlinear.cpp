#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "lcdimlinear.h"
#include "endcaps.h"
#include <cad/tools/string_helper.h>

#include <cmath>

using namespace lc::viewer;

LCDimLinear::LCDimLinear(const lc::entity::DimLinear_CSPtr& dimLinear) :
    LCVDrawItem(dimLinear, true),
    _dimLinear(dimLinear) {
}

/**
* Draw a DimLinear
* The dimension line runs through the definition point at the dimension's
* angle (DXF group 50), and the measurement is the distance between the two
* measured points along it, as LibreCAD 2 and other CAD programs draw it.
* TODO: When oblique is set, draw the extension lines at that angle
*       see http://www.cad-notes.com/autocad-isometric-text-and-dimension/
*/
void LCDimLinear::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    const double capSize = 10.;

    const lc::geo::Coordinate along(std::cos(_dimLinear->angle()), std::sin(_dimLinear->angle()));
    const lc::geo::Coordinate across(-along.y(), along.x());
    const lc::geo::Coordinate& onLine = _dimLinear->definitionPoint();
    const lc::geo::Coordinate& p2 = _dimLinear->definitionPoint2();
    const lc::geo::Coordinate& p3 = _dimLinear->definitionPoint3();

    // Where each measured point's extension line meets the dimension line.
    const lc::geo::Coordinate foot2 = p2 + across * across.dot(onLine - p2);
    const lc::geo::Coordinate foot3 = p3 + across * across.dot(onLine - p3);
    const double measured = std::abs(along.dot(p3 - p2));

    // Decide to show the explicit value or the measured value
    std::string value = lc::tools::StringHelper::dim_value(
                            _dimLinear->explicitValue(),
                            options.linearFormat(),
                            measured
                        );

    // get text size
    painter.save();
    painter.font_size(options.dimTextHeight(), false);
    TextExtends te = painter.text_extends(value.c_str());
    painter.restore();

    // Draw line
    EndCaps endCaps;
    const bool shortOnSpace = (measured - te.width - capSize * 2) < 0;

    if (shortOnSpace) {
        // Arrows outside the extension lines, pointing in at them.
        const lc::geo::Coordinate outward = along * (along.dot(foot3 - foot2) >= 0. ? capSize * 2 : -capSize * 2);
        const lc::geo::Coordinate out3 = foot3 + outward;
        const lc::geo::Coordinate out2 = foot2 - outward;

        painter.move_to(p3.x(), p3.y());
        painter.line_to(foot3.x(), foot3.y());
        painter.line_to(out3.x(), out3.y());
        painter.move_to(out2.x(), out2.y());
        painter.line_to(foot2.x(), foot2.y());
        painter.line_to(p2.x(), p2.y());
        painter.stroke();

        endCaps.render(painter, EndCaps::CLOSEDARROW, out3.x(), out3.y(), foot3.x(), foot3.y(), capSize);
        endCaps.render(painter, EndCaps::CLOSEDARROW, out2.x(), out2.y(), foot2.x(), foot2.y(), capSize);
    }
    else {
        painter.move_to(p3.x(), p3.y());
        painter.line_to(foot3.x(), foot3.y());
        painter.line_to(foot2.x(), foot2.y());
        painter.line_to(p2.x(), p2.y());
        painter.stroke();

        endCaps.render(painter, EndCaps::CLOSEDARROW, foot3.x(), foot3.y(), foot2.x(), foot2.y(), capSize);
        endCaps.render(painter, EndCaps::CLOSEDARROW, foot2.x(), foot2.y(), foot3.x(), foot3.y(), capSize);
    }

    // The text runs along the dimension line, turned so it never reads upside
    // down: a vertical dimension reads from the bottom up.
    double textAngle = std::remainder(_dimLinear->angle(), 2. * M_PI);
    if (textAngle > M_PI / 2.) {
        textAngle -= M_PI;
    }
    else if (textAngle <= -M_PI / 2.) {
        textAngle += M_PI;
    }

    this->drawText(
        value,
        textAngle + _dimLinear->textAngle(),
        _dimLinear->attachmentPoint(),
        _dimLinear->middleOfText(),
        painter,
        options,
        rect
    );
}

lc::entity::CADEntity_CSPtr LCDimLinear::entity() const {
    return _dimLinear;
}
