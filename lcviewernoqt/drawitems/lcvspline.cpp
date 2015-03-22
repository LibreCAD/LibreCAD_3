#include "lcvspline.h"
#include "../lcpainter.h"
#include "lcdrawoptions.h"

LCVSpline::LCVSpline(const lc::Spline_CSPtr spline) : LCVDrawItem(true), lc::Spline(spline, true) {
}

void LCVSpline::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {



}

