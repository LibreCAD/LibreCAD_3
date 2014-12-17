#include "lcvarc.h"
#include "../lcpainter.h"
#include "lcdrawoptions.h"

LCVArc::LCVArc(const lc::Arc_CSPtr arc) : LCVDrawItem(true), lc::Arc(arc, true) {
}

void LCVArc::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {
    bool modified = false;

    if (radius() /** painter->scale() > 5 */) {
        painter->arc(center().x(), center().y(), radius(), startAngle(), endAngle());
        painter->stroke();
    }

    if (modified) {
        painter->restore();
    }
}

