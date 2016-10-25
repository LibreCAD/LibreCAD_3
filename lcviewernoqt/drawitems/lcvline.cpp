#include "lcvline.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "cad/geometry/geovector.h"
#include "cad/math/intersectionhandler.h"
#include "cad/geometry/geocircle.h"
#include "cad/geometry/geobeziercubic.h"
#include "cad/geometry/geocircle.h"
#include "cad/geometry/geobezierbase.h"

using namespace LCViewer;
LCVLine::LCVLine(const lc::entity::Line_CSPtr line) : LCVDrawItem(true), lc::entity::Line(line, true) {
}

void LCVLine::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    painter.move_to(start().x(), start().y());
    painter.line_to(end().x(), end().y());
    painter.stroke();
}
