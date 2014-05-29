#include "lcvellipse.h"
#include "lcpainter.h"
#include "lcdrawoptions.h"
#include <QColor>
LCVEllipse::LCVEllipse(const lc::Ellipse_CSPtr Ellipse) : LCVDrawItem(true), lc::Ellipse(Ellipse, true) {
}

void LCVEllipse::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {

    bool modified = false;

    if (this->selected()) {
        modified = true;
        painter->save();
        painter->source_rgba(
            options->selectedColor().redF(),
            options->selectedColor().greenF(),
            options->selectedColor().blueF(),
            options->selectedColor().alphaF()
        );
    }

    if (minorRadius() /** painter->scale() > 5 */) {
//        painter->circle(center().x(), center().y(), radius());
        //painter->ellipse(center().x(), center().y(), majorRadius(), minorRadius() ,startAngle(),endAngle());
        //painter->stroke();
    }

    if (modified) {
        painter->restore();
    }

}
