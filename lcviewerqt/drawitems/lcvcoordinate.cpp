#include "lcvcoordinate.h"
#include "lcpainter.h"
#include "lcdrawoptions.h"
#include <QDebug>
LCVCoordinate::LCVCoordinate(const lc::Coordinate_CSPtr Coordinate) : LCVDrawItem(true), lc::Coordinate(Coordinate, true) {
}

void LCVCoordinate::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {
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

    painter->point(x(), y(), 3., true);
        painter->stroke();
    if (modified) {
        painter->restore();
    }
}

