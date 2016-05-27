#include "lcvline.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"

// #include "cad/geometry/geobezier.h"

using namespace LCViewer;
LCVLine::LCVLine(const lc::entity::Line_CSPtr line) : LCVDrawItem(true), lc::entity::Line(line, true) {
}

void LCVLine::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {

    painter.move_to(start().x(), start().y());
    painter.line_to(end().x(), end().y());
    painter.stroke();


    /* Nearest point on entitty for bezier

    auto p1 = lc::geo::Coordinate(1800,2400);
    auto p2 = lc::geo::Coordinate(2,2);
    auto p3 = lc::geo::Coordinate(2200, 1800);

    auto bb = lc::geo::Bezier(p1,p2,p3);
    auto pt = lc::geo::Coordinate{1900, 3000};

    auto nearest = bb.nearestPointOnPath(pt);

    painter.move_to(p1.x(), p1.y());
    painter.quadratic_curve_to(p2.x(), p2.y(), p3.x(), p3.y());

    painter.move_to(pt.x(), pt.y());
    painter.line_to(nearest.x(), nearest.y());
    painter.stroke();

    */
}

