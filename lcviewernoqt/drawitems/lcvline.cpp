#include "lcvline.h"
#include "../lcpainter.h"
#include "lcdrawoptions.h"

LCVLine::LCVLine(const lc::Line_CSPtr line) : LCVDrawItem(true), lc::Line(line, true) {
}

void LCVLine::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {



    painter->move_to(start().x(), start().y());
    painter->line_to(end().x(), end().y());
    painter->stroke();


}

