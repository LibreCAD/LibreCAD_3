#include "lcvspline.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"

LCVSpline::LCVSpline(const lc::entity::Spline_CSPtr spline) : LCVDrawItem(true), lc::entity::Spline(spline, true) {
}

void LCVSpline::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {



}

