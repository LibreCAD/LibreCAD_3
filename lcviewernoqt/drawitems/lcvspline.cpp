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

    auto bezlist = beziers();

    for(const auto &bez: bezlist) {
        painter.move_to(bez[0].x(), bez[0].y());

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
