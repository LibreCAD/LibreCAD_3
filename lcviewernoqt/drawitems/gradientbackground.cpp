#include "gradientbackground.h"

#include <cairo.h>
#include "../lcpainter.h"
#include "cad/geometry/geoarea.h"
#include "cad/meta/color.h"
GradientBackground::GradientBackground(const lc::Color& topColor, const lc::Color& bottomColor) : LCVDrawItem(false), _topColor(topColor), _bottomColor(bottomColor) {
}

GradientBackground::~GradientBackground() {
}

void GradientBackground::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& updateRect) const {
    unsigned long patId = painter.pattern_create_linear(0.0,  updateRect.minP().y(),  0.0, updateRect.maxP().y());

    painter.pattern_add_color_stop_rgba(patId, 1., _bottomColor.red(), _bottomColor.green(), _bottomColor.blue(), _bottomColor.alpha());
    painter.pattern_add_color_stop_rgba(patId, 0., _topColor.red(), _topColor.green(), _topColor.blue(), _topColor.alpha());
    painter.rectangle(updateRect.minP().x(), updateRect.minP().y(), updateRect.width(), updateRect.height());
    painter.set_source(patId);
    painter.fill();
    painter.pattern_destroy(patId);
}
