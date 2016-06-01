#include "lcvline.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"

#include "cad/geometry/geovector.h"
#include "cad/geometry/geobezier.h"
 #include "cad/geometry/geocircle.h"
#include "cad/math/intersectionhandler.h"
#include "cad/geometry/geoarc.h"
using namespace LCViewer;
LCVLine::LCVLine(const lc::entity::Line_CSPtr line) : LCVDrawItem(true), lc::entity::Line(line, true) {
}

void LCVLine::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {

    painter.move_to(start().x(), start().y());
    painter.line_to(end().x(), end().y());
    painter.stroke();


    /*
    auto c1 = lc::geo::Coordinate(1000, -1000);
    auto c2 = lc::geo::Coordinate(-1000, 1000);
    //    auto circle_ = lc::geo::Arc(c1, 200., 90*3.14/180, 270*3.14/180);

    auto circle_ = lc::geo::Vector(c1,c2);

    auto p1 = lc::geo::Coordinate(200,300);
    auto p2 = lc::geo::Coordinate(0,-900);
    auto p3 = lc::geo::Coordinate(-200,300);

    auto bezier_ = lc::geo::Bezier(p1,p2,p3);

    auto ret = lc::maths::Intersection::BezierLine(bezier_, circle_);

    painter.move_to(0,0);
//    painter.circle(c1.x(), c1.y(), 200.);

    painter.move_to(c1.x(), c1.y());
    painter.line_to(c2.x(), c2.y());
    painter.stroke();

//    painter.arc(c1.x(), c1.y(), 200.,  90*3.14/180, 270*3.14/180);

    painter.move_to(p1.x(), p1.y());
    painter.quadratic_curve_to(p2.x(), p2.y(), p3.x(), p3.y());
    painter.stroke();

    for(auto r : ret) {
        painter.point(r.x(), r.y(), 10, 1);
    }

    painter.stroke();
*/

    /* Nearest point on entitty for bezier

    auto p1 = lc::geo::Coordinate(1800,2400);
    auto p2 = lc::geo::Coordinate(2,2);
    auto p3 = lc::geo::Coordinate(2200, 1800);

    auto p1 = lc::geo::Coordinate(0,0);
    auto p2 = lc::geo::Coordinate(0,250);
    auto p3 = lc::geo::Coordinate(250,250);


    auto bb = lc::geo::Bezier(p1,p2,p3);
    auto pt = lc::geo::Coordinate{1900, 3000};

    auto nearest = bb.nearestPointOnPath(pt);

    painter.move_to(p1.x(), p1.y());
    painter.quadratic_curve_to(p2.x(), p2.y(), p3.x(), p3.y());

    for(double i = 0.; i < 1.; i = i + 0.1) {
        auto start = bb.DirectValueAt(i);
        auto end = bb.normal(i);
        painter.move_to(start.x(), start.y());
        painter.line_to(end.x(), end.y());
        painter.stroke();
    }

    painter.move_to(pt.x(), pt.y());
    painter.line_to(nearest.x(), nearest.y());
    painter.stroke();

    */
}

