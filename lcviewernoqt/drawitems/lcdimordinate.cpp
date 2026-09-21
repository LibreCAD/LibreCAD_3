#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "lcdimordinate.h"
#include <cad/const.h>
#include <cad/tools/string_helper.h>

#include <cmath>

using namespace lc::viewer;

namespace {

/**
 * The attachment point drawText() needs to put the text where the given one
 * says it goes.
 *
 * The two read an attachment point the opposite way round. This dimension's is
 * DXF's -- group 71, the point of the text that sits at middleOfText, so
 * Bottom_center puts the text above it -- and drawText() takes it as the side of
 * the location the text goes on, so there Top_center puts the text above.
 */
lc::TextConst::AttachmentPoint sideTheTextGoesOn(lc::TextConst::AttachmentPoint attachmentPoint) {
    switch (attachmentPoint) {
    case lc::TextConst::AttachmentPoint::Top_left:
        return lc::TextConst::AttachmentPoint::Bottom_right;

    case lc::TextConst::AttachmentPoint::Top_center:
        return lc::TextConst::AttachmentPoint::Bottom_center;

    case lc::TextConst::AttachmentPoint::Top_right:
        return lc::TextConst::AttachmentPoint::Bottom_left;

    case lc::TextConst::AttachmentPoint::Middle_left:
        return lc::TextConst::AttachmentPoint::Middle_right;

    case lc::TextConst::AttachmentPoint::Middle_right:
        return lc::TextConst::AttachmentPoint::Middle_left;

    case lc::TextConst::AttachmentPoint::Bottom_left:
        return lc::TextConst::AttachmentPoint::Top_right;

    case lc::TextConst::AttachmentPoint::Bottom_center:
        return lc::TextConst::AttachmentPoint::Top_center;

    case lc::TextConst::AttachmentPoint::Bottom_right:
        return lc::TextConst::AttachmentPoint::Top_left;

    default:
        return attachmentPoint;
    }
}

}  // namespace

LCDimOrdinate::LCDimOrdinate(const lc::entity::DimOrdinate_CSPtr& dimOrdinate) :
    LCVDrawItem(dimOrdinate, true),
    _dimOrdinate(dimOrdinate) {
}

void LCDimOrdinate::draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const {
    const lc::geo::Coordinate& feature = _dimOrdinate->featurePoint();
    const lc::geo::Coordinate& leaderEnd = _dimOrdinate->leaderEndPoint();
    const bool xType = _dimOrdinate->xType();

    // Like AutoCAD, the ordinate is shown without its sign; which side of the
    // datum the feature is on is plain from the drawing.
    std::string value = lc::tools::StringHelper::dim_value(
                            _dimOrdinate->explicitValue(),
                            options.linearFormat(),
                            std::abs(_dimOrdinate->value())
                        );

    // The leader leaves the feature at right angles to the measured axis:
    // vertically for an X ordinate, horizontally for a Y one. `along` is how
    // far it runs that way, `aside` how far its end is off to the side.
    const double along = xType ? leaderEnd.y() - feature.y() : leaderEnd.x() - feature.x();
    const double aside = xType ? leaderEnd.x() - feature.x() : leaderEnd.y() - feature.y();
    const auto fromFeature = [&](double alongBy, double asideBy) {
        return xType ? lc::geo::Coordinate(feature.x() + asideBy, feature.y() + alongBy)
                     : lc::geo::Coordinate(feature.x() + alongBy, feature.y() + asideBy);
    };

    painter.move_to(feature.x(), feature.y());

    // An end off to the side is reached with a jog: straight out for a third
    // of the way, across to the end's line over the next third, then straight
    // on to the end.
    if (std::abs(aside) > LCTOLERANCE) {
        const auto firstBend = fromFeature(along / 3., 0.);
        const auto secondBend = fromFeature(along * 2. / 3., aside);
        painter.line_to(firstBend.x(), firstBend.y());
        painter.line_to(secondBend.x(), secondBend.y());
    }

    painter.line_to(leaderEnd.x(), leaderEnd.y());
    painter.stroke();

    this->drawText(
        value,
        _dimOrdinate->textAngle(),
        sideTheTextGoesOn(_dimOrdinate->attachmentPoint()),
        _dimOrdinate->middleOfText(),
        painter,
        options,
        rect
    );
}

lc::entity::CADEntity_CSPtr LCDimOrdinate::entity() const {
    return _dimOrdinate;
}
