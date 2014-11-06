#include "lcvcoordinate.h"
#include "../lcpainter.h"
#include "lcdrawoptions.h"
LCVCoordinate::LCVCoordinate(const lc::Coordinate_CSPtr Coordinate) : LCVDrawItem(true), lc::Coordinate(Coordinate, true) {
}

void LCVCoordinate::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {
    bool modified = false;

    if (this->selected()) {
        modified = true;
        painter->save();
        painter->source_rgba(
            options->selectedColor().red(),
            options->selectedColor().green(),
            options->selectedColor().blue(),
            options->selectedColor().alpha()
        );
    }

    painter->point(x(), y(), 3., true);
    painter->stroke();

    if (modified) {
        painter->restore();
    }
}

