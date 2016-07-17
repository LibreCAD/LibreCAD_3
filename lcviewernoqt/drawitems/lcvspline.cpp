#include "lcvspline.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"
#include <vector>
#include "cad/geometry/geocoordinate.h"
#include <iostream>

using namespace LCViewer;

LCVSpline::LCVSpline(const lc::entity::Spline_CSPtr spline) : LCVDrawItem(true), lc::entity::Spline(spline, true) {
}

void LCVSpline::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {

    std::cout << "DEBUG DRAWING";

//    auto controlPoints = this->controlPoints();
//    if (controlPoints.size()<2) return;

//    auto end = controlPoints.end();
//    std::vector<lc::geo::Coordinate>::iterator it = controlPoints.begin();
//    lc::geo::Coordinate lastPoint = *it, thisCoord = *it;
//    painter.move_to(lastPoint.x(), lastPoint.y());
//    it++;
//    while (it != end) {
//        thisCoord = *it;
//        auto c = (lastPoint + thisCoord) / 2.;
//        painter.quadratic_curve_to(lastPoint.x(), lastPoint.y(), c.x(), c.y());
//        lastPoint = thisCoord;
//        it++;
//    }
//    // TODO handle flags
//    painter.quadratic_curve_to(lastPoint.x(), lastPoint.y(), thisCoord.x(), thisCoord.y());
//    painter.stroke();

    std::vector<std::vector<lc::geo::Coordinate>>bezlist = getBeziers();

    std::cout <<"Bezies called";
//    painter.move_to(bezlist[0][0].x(), bezlist[0][0].y());

//    lc::geo::Coordinate l = bezlist.at(0).at(0);

//    std::cout << bezlist.size() << bezlist.at(0).size();

    if(bezlist.size()>0) {
        auto bez = bezlist.at(0);
        if(bez.size()>0) {
            painter.move_to(bez.at(0).x(), bez.at(0).y());
        }
    }

    for(const auto &bez: bezlist) {
        if(bez.size()==4) {
            painter.curve_to(bez[1].x(), bez[1].y(), bez[2].x(), bez[2].y(), bez[3].x(), bez[3].y());
        } else if (bez.size()==3) {
            painter.quadratic_curve_to(bez[1].x(), bez[1].y(), bez[2].x(), bez[2].y());
        } else if(bez.size()==2) {
            painter.line_to(bez[1].x(), bez[1].y());
        }
    }
    painter.stroke();
}

