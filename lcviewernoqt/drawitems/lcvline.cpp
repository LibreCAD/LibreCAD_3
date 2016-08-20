#include "lcvline.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"
#include "cad/geometry/geovector.h"

using namespace LCViewer;
LCVLine::LCVLine(const lc::entity::Line_CSPtr line) : LCVDrawItem(true), lc::entity::Line(line, true) {
}

void LCVLine::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
//    painter.move_to(start().x(), start().y());
//    painter.line_to(end().x(), end().y());
//    painter.stroke();

//    painter.move_to(-1000,0);
//    painter.line_to(1000,0);
    painter.circle(200,0, 150);
    painter.stroke();

    painter.move_to(100,-500);
    painter.curve_to(300,1500, 500, -1500, 1000, 500);
    painter.stroke();


//    auto p1 = lc::geo::Coordinate(100,-500);
//    auto p2 = lc::geo::Coordinate(300,500);
//    auto p3 = lc::geo::Coordinate(600,-500);
//    auto p4 = lc::geo::Coordinate(1000,500);

}

