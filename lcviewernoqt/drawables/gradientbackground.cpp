#include "gradientbackground.h"

using namespace lc;
using namespace lc::viewer::drawable;

GradientBackground::GradientBackground(const lc::Color& topColor, const lc::Color& bottomColor) : _topColor(topColor), _bottomColor(bottomColor) {
}

void GradientBackground::draw(event::DrawEvent const & event) const {
    LcPainter &painter = event.painter();
    const lc::geo::Area &updateRect = event.updateRect();
    unsigned long patId = painter.pattern_create_linear(0.0,  updateRect.minP().y(),  0.0, updateRect.maxP().y());
    painter.pattern_add_color_stop_rgba(patId, 0., _topColor.red(), _topColor.green(), _topColor.blue(), _topColor.alpha());
    painter.pattern_add_color_stop_rgba(patId,  1., _bottomColor.red(), _bottomColor.green(), _bottomColor.blue(), _bottomColor.alpha());
    painter.set_pattern_source(patId);
    painter.rectangle(updateRect.minP().x(), updateRect.minP().y(), updateRect.width(), updateRect.height());
    painter.fill();
    painter.pattern_destroy(patId);
}
