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

    auto c1 = lc::geo::Coordinate(0, 0);
    auto rad = 100;
    auto ell= lc::geo::Circle(c1, rad);

    auto p1 = lc::geo::Coordinate(200,300);
    auto p2 = lc::geo::Coordinate(0,-900);
    auto p3 = lc::geo::Coordinate(-200,300);
    auto p4 = lc::geo::Coordinate(-600,600);

    auto bez = lc::geo::CubicBezier(p1,p2,p3, p4);
    lc::geo::BB_CSPtr B = std::make_shared<const lc::geo::CubicBezier>(bez);

    auto bb2 = B->boundingBox();
    lc::geo::Area cc2 = lc::geo::Area(lc::geo::Coordinate(ell.center().x() - ell.radius(), ell.center().y() - ell.radius()),
                     lc::geo::Coordinate(ell.center().x() + ell.radius(), ell.center().y() + ell.radius()));

    for(auto p : B->getCP()) {
        painter.point(p.x(), p.y(), 5, 1);
    }

    painter.rectangle(bb2.minP().x(), bb2.minP().y(), bb2.maxP().x()-bb2.minP().x(), bb2.maxP().y()-bb2.minP().y());
    painter.rectangle(cc2.minP().x(), cc2.minP().y(), cc2.maxP().x()-cc2.minP().x(), cc2.maxP().y()-cc2.minP().y());
//    painter.stroke();

    painter.move_to(p1.x(), p1.y());
    painter.curve_to(p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y());
    painter.stroke();

    auto ret = lc::maths::Intersection::BezierCircle(B, ell);

    for(auto r : ret) {
        painter.point(r.x(), r.y(), 5, 1);
    }
    painter.stroke();
    painter.circle(ell.center().x(), ell.center().y(), ell.radius());
    painter.stroke();
}

