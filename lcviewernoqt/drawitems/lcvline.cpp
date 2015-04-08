#include "lcvline.h"
#include "../lcpainter.h"
#include "lcdrawoptions.h"

LCVLine::LCVLine(const lc::entity::Line_CSPtr line) : LCVDrawItem(true), lc::entity::Line(line, true) {
}

void LCVLine::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {



    painter.move_to(start().x(), start().y());
    painter.line_to(end().x(), end().y());
    painter.stroke();


}

