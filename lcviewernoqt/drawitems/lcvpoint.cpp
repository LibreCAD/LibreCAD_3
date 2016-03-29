#include "lcvpoint.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"
using namespace LCViewer;

LCVPoint::LCVPoint(const lc::entity::Point_CSPtr Coordinate) : LCVDrawItem(true), lc::entity::Point (Coordinate, true) {
}

void LCVPoint::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    bool modified = false;



    painter.point(x(), y(), 3., true);
    painter.stroke();

    if (modified) {
        painter.restore();
    }
}

