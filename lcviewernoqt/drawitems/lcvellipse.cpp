#include "lcvellipse.h"
#include "../lcpainter.h"
#include "lcdrawoptions.h"
LCVEllipse::LCVEllipse(const lc::entity::Ellipse_CSPtr Ellipse) : LCVDrawItem(true), lc::entity::Ellipse(Ellipse, true) {
}

void LCVEllipse::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {

    bool modified = false;

    if (minorRadius() /** painter->scale() > 5 */) {
        //        painter->circle(center().x(), center().y(), radius());
        painter->ellipse(center().x(), center().y(), majorRadius(), minorRadius() , startAngle(), endAngle(), getAngle());
        painter->stroke();
    }

    if (modified) {
        painter->restore();
    }

}
