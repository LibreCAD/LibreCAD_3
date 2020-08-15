#include "lcvline.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "cad/geometry/geovector.h"
#include "cad/math/intersectionhandler.h"
#include "cad/geometry/geocircle.h"
#include "cad/geometry/geobeziercubic.h"
#include "cad/geometry/geocircle.h"
#include "cad/geometry/geobezierbase.h"

using namespace lc::viewer;

LCVLine::LCVLine(const lc::entity::Line_CSPtr& line) :
    LCVDrawItem(line, true),
    _line(line) {
}

void LCVLine::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    painter.move_to(_line->start().x(), _line->start().y());
    painter.line_to(_line->end().x(), _line->end().y());
    if(autostroke())
        painter.stroke();
}

lc::entity::CADEntity_CSPtr LCVLine::entity() const {
    return _line;
}
