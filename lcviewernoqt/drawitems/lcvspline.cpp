#include "lcvspline.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"
using namespace LCViewer;

LCVSpline::LCVSpline(const lc::entity::Spline_CSPtr spline) : LCVDrawItem(true), lc::entity::Spline(spline, true) {
}

void LCVSpline::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {

    auto controlPoints = this->controlPoints();
    auto end = controlPoints.end();
    std::vector<lc::geo::Coordinate>::iterator it = controlPoints.begin();
    lc::geo::Coordinate &lastPoint=*it, &thisCoord = *it;
    painter.move_to(lastPoint.x(), lastPoint.y());
    it++;
    while (it!=end) {
        thisCoord = *it;
        auto xc = (lastPoint.x() + thisCoord.x()) / 2.;
        auto yc = (lastPoint.y() + thisCoord.y()) / 2.;
        painter.quadratic_curve_to(lastPoint.x(), lastPoint.y(), xc, yc);
        lastPoint = thisCoord;
        it++;
    }
    painter.quadratic_curve_to(lastPoint.x(), lastPoint.y(), thisCoord.x(), thisCoord.y());
    painter.stroke();

}

