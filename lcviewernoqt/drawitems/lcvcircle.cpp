#include "lcvcircle.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"
using namespace LCViewer;
LCVCircle::LCVCircle(const lc::entity::Circle_CSPtr circle) : LCVDrawItem(true), lc::entity::Circle(circle, true) {
}

void LCVCircle::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    bool modified = false;

    if (radius() /** painter.scale() > 5 */) {
        painter.circle(center().x(), center().y(), radius());
        painter.stroke();
    }

    if (modified) {
        painter.restore();
    }
}

