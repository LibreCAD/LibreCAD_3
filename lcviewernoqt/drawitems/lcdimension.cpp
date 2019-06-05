#include "lcdimension.h"

#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "endcaps.h"
#include <cad/tools/string_helper.h>
#include <cad/math/lcmath.h>
#include <cad/const.h>

using namespace lc::viewer;

void LCDimension::drawText(std::string const& value,
                           double angle,
                           lc::TextConst::AttachmentPoint const attachmentPoint,
                           const lc::geo::Coordinate& textlocation,
                           LcPainter& painter,
                           const LcDrawOptions& options,
                           const lc::geo::Area& rect) const {
    painter.save();
    double height = options.dimTextHeight();
    painter.font_size(height, false);

    //    double alignX, alignY;
    // The idea of height * .2 is just a average basline offset. Don't this value to seriously,
    // we could get it from font exists but that sounds over exaggerating for the moment.

    lc::TextConst::AttachmentPoint aPoint = attachmentPoint;
    double newAngle = angle;

    // THis code 'fixes' the Atatchment point if the text is behind some rotation
    if (angle > 0.5 * M_PI || angle < -0.5 * M_PI) {
        newAngle = angle - M_PI;

        switch (attachmentPoint) {
            case lc::TextConst::AttachmentPoint::Top_left:
                aPoint = lc::TextConst::AttachmentPoint::Top_right;
                break;

            case lc::TextConst::AttachmentPoint::Top_center:
                aPoint = lc::TextConst::AttachmentPoint::Bottom_center;
                break;

            case lc::TextConst::AttachmentPoint::Top_right:
                aPoint = lc::TextConst::AttachmentPoint::Bottom_left;
                break;

            case lc::TextConst::AttachmentPoint::Middle_left:
                aPoint = lc::TextConst::AttachmentPoint::Middle_right;
                break;

            case lc::TextConst::AttachmentPoint::Middle_right:
                aPoint = lc::TextConst::AttachmentPoint::Middle_left;
                break;

            case lc::TextConst::AttachmentPoint::Bottom_left:
                aPoint = lc::TextConst::AttachmentPoint::Top_right;
                break;

            case lc::TextConst::AttachmentPoint::Bottom_center:
                aPoint = lc::TextConst::AttachmentPoint::Top_center;
                break;

            case lc::TextConst::AttachmentPoint::Bottom_right:
                aPoint = lc::TextConst::AttachmentPoint::Top_left;
                break;

            default: // Top Center
                break;
        }
    }


    TextExtends te = painter.text_extends(value.c_str());
    double alignY, alignX;

    switch (aPoint) {
        case lc::TextConst::AttachmentPoint::Top_left:
            alignY = 0.0 + (height * .2);
            alignX = - te.width;
            break;

        case lc::TextConst::AttachmentPoint::Top_center:
            alignY = 0.0 + (height * .2);
            alignX = - te.width / 2.0;
            break;

        case lc::TextConst::AttachmentPoint::Top_right:
            alignY = 0.0 + (height * .2);
            alignX = 0.;
            break;

        case lc::TextConst::AttachmentPoint::Middle_left:
            alignY = -height / 2. + (height * .2);
            alignX = - te.width;
            break;

        case lc::TextConst::AttachmentPoint::Middle_center:
            alignY = -height / 2. + (height * .2);
            alignX = - te.width / 2.0;
            break;

        case lc::TextConst::AttachmentPoint::Middle_right:
            alignY = -height / 2. + (height * .2);
            alignX = 0.;
            break;

        case lc::TextConst::AttachmentPoint::Bottom_left:
            alignY = -height + (height * .2);
            alignX = - te.width;
            break;

        case lc::TextConst::AttachmentPoint::Bottom_center:
            alignY = -height + (height * .2);
            alignX = - te.width / 2.0;
            break;

        case lc::TextConst::AttachmentPoint::Bottom_right:
            alignY = -height + (height * .2);
            alignX = 0.;
            break;

        default: // Top Center
            alignY = 0.0 + (height * .2);
            alignX = - te.width / 2.0;
            break;
    }

     painter.translate(textlocation.x(), -textlocation.y());
     painter.rotate(-newAngle);
     painter.translate(alignX, -alignY);
     painter.move_to(0., 0.);
     painter.text(value.c_str());
     painter.stroke();
     painter.restore();
}
