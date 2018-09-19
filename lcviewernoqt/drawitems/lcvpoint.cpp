#include "lcvpoint.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
using namespace LCViewer;

LCVPoint::LCVPoint(const lc::entity::Point_CSPtr& coordinate) :
        LCVDrawItem(coordinate, true),
        _point(coordinate) {
}

void LCVPoint::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    painter.point(_point->x(), _point->y(), 3., true);
    painter.stroke();
}

lc::entity::CADEntity_CSPtr LCVPoint::entity() const {
    return _point;
}

