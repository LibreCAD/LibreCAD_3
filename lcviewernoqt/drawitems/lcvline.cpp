#include "lcvline.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"
#include <iostream>
#include "cad/geometry/geovector.h"
#include "cad/geometry/geobezier.h"
#include "cad/geometry/geobezierbase.h"
#include "cad/geometry/geobeziercubic.h"
#include "cad/geometry/geocircle.h"
#include "cad/math/intersectionhandler.h"
#include "cad/geometry/geoarc.h"
#include "cad/geometry/geoellipse.h"
using namespace LCViewer;
LCVLine::LCVLine(const lc::entity::Line_CSPtr line) : LCVDrawItem(true), lc::entity::Line(line, true) {
}

void LCVLine::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {

    painter.move_to(start().x(), start().y());
    painter.line_to(end().x(), end().y());
    painter.stroke();

//    auto r = lc::geo::Coordinate(100, 100);
//    painter.point(r.x(), r.y(), 5, 1);

//    auto z = r.mirror(start(), end());

//    painter.point(z.x(), z.y(), 5, 1);
//    painter.stroke();

    auto c1 = lc::geo::Coordinate(0, 0);
//    auto mp = lc::geo::Coordinate(1000, 0);
    auto rad = 200;
//    auto sa = 0;
//    auto ea = 3.14 * 2;
    auto ell= lc::geo::Circle(c1, rad);

//    auto ellipse = ell;
//    auto E = ell;
    auto p1 = lc::geo::Coordinate(200,300);
    auto p2 = lc::geo::Coordinate(0,-900);
    auto p3 = lc::geo::Coordinate(-200,300);
    auto p4 = lc::geo::Coordinate(-600,600);
    auto bez = lc::geo::CubicBezier(p1,p2,p3, p4);
    lc::geo::BB_CSPtr B = std::make_shared<const lc::geo::CubicBezier>(bez);

//    auto bp1 = lc::geo::Coordinate(200,300);
//    auto bp2 = lc::geo::Coordinate(-900,0);
//    auto bp3 = lc::geo::Coordinate(200,-300);
//    auto bp4 = lc::geo::Coordinate(-600,600);
//    auto bez2 = lc::geo::CubicBezier(bp1,bp2,bp3, bp4);
//    lc::geo::BB_CSPtr B2 = std::make_shared<const lc::geo::CubicBezier>(bez2);


    painter.move_to(p1.x(), p1.y());
    painter.curve_to(p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y());
    painter.stroke();

//    painter.move_to(bp1.x(), bp1.y());
//    painter.curve_to(bp2.x(), bp2.y(), bp3.x(), bp3.y(), bp4.x(), bp4.y());
//    painter.stroke();

//    auto ret = lc::maths::Intersection::BezierBezier(B, B2);

//    for(auto r : ret) {
//        painter.point(r.x(), r.y(), 5, 1);
//    }
//    painter.stroke();

//    painter.move_to(bp1.x(), bp1.y());
//    painter.quadratic_curve_to(bp2.x(), bp2.y(), bp3.x(), bp3.y());
//    painter.stroke();

//    auto B2 = lc::geo::Bezier(bp1,bp2,bp3);




    //    auto bezier_ = lc::geo::Bezier(p1,p2,p3);
//    auto B = lc::geo::Bezier(p1,p2,p3);

//    auto ret = lc::maths::Intersection::BezierEllipse(bezier_, ellipse);

//    for(auto r : ret) {
//        painter.point(r.x(), r.y(), 10, 1);
//    }
//    painter.stroke();

//    painter.ellipse(ell.center().x(), ell.center().y(), ell.majorRadius(), ell.minorRadius(), ell.startAngle(), ell.endAngle(), ell.getAngle());
//    painter.circle(ell.center().x(), ell.center().y(), ell.radius());
//    painter.stroke();

//    painter.move_to(p1.x(), p1.y());
//    painter.quadratic_curve_to(p2.x(), p2.y(), p3.x(), p3.y());
//    painter.stroke();

//    using namespace lc;
//    auto C = geo::Ellipse(E.center() - E.center(), E.majorP(), E.minorRadius(), E.startAngle(), E.endAngle())
//            .georotate(geo::Coordinate(0,0), -E.getAngle())
//            .geoscale(geo::Coordinate(0,0), geo::Coordinate(1/E.ratio(), 1));

//    auto Bez = B
//            .move(-E.center()).rotate(geo::Coordinate(0,0), E.getAngle())
//            .scale(geo::Coordinate(0,0), geo::Coordinate(1/E.ratio(), 1));

//    painter.move_to(0,0);

//    painter.ellipse(C.center().x(), C.center().y(), C.majorRadius(), C.minorRadius(), C.startAngle(), C.endAngle(), C.getAngle());
//    painter.stroke();

//    painter.move_to(Bez.pointA().x(), Bez.pointA().y());
//    painter.quadratic_curve_to(Bez.pointB().x(), Bez.pointB().y(), Bez.pointC().x(), Bez.pointC().y());
//    painter.stroke();

    /*
    auto c1 = lc::geo::Coordinate(1000, -1000);
    auto c2 = lc::geo::Coordinate(-1000, 1000);
    //    auto circle_ = lc::geo::Arc(c1, 200., 90*3.14/180, 270*3.14/180);

    auto circle_ = lc::geo::Vector(c1,c2);


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

