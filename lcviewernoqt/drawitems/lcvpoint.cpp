#include "lcvpoint.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"

using namespace lc::viewer;

LCVPoint::LCVPoint(const lc::entity::Point_CSPtr& coordinate) :
    LCVDrawItem(coordinate, true),
    _point(coordinate) {
}

void LCVPoint::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    painter.point(_point->x(), _point->y(), 3., true); //This actually strokes circle
    painter.fill(); //fill method is not working in openGL painter
}

lc::entity::CADEntity_CSPtr LCVPoint::entity() const {
    return _point;
}

