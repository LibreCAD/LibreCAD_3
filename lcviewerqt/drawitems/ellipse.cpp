#include "lcvellipse.h"
#include "lcpainter.h"
#include "lcdrawoptions.h"
#include <QColor>
LCVEllipse::LCVEllipse(const lc::Ellipse_CSPtr ellipse) : LCVDrawItem(true), lc::Ellipse(ellipse, true) {
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

    painter->move_to(start().x(), start().y());
    painter->line_to(end().x(), end().y());
    painter->stroke();

    if (modified) {
        painter->restore();
    }

}

