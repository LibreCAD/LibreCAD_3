#include "lcvellipse.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
using namespace LCViewer;

LCVEllipse::LCVEllipse(const lc::entity::Ellipse_CSPtr Ellipse) : LCVDrawItem(true), lc::entity::Ellipse(Ellipse, true) {
}

void LCVEllipse::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {

    bool modified = false;

    if (minorRadius() /** painter.scale() > 5 */) {
        //        painter.circle(center().x(), center().y(), radius());
        painter.ellipse(center().x(), center().y(), majorRadius(), minorRadius() , startAngle(), endAngle(), getAngle());
        painter.stroke();
    }

    if (modified) {
        painter.restore();
    }

}
