#include "lcvcircle.h"
#include "../lcpainter.h"
#include "lcdrawoptions.h"

LCVCircle::LCVCircle(const lc::Circle_CSPtr circle) : LCVDrawItem(true), lc::Circle(circle, true) {
}

void LCVCircle::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {
    bool modified = false;

    if (radius() /** painter->scale() > 5 */) {
        painter->circle(center().x(), center().y(), radius());
        painter->stroke();
    }

    if (modified) {
        painter->restore();
    }
}

