#include "gradientbackground.h"

#include <QDebug>

#include <cairo.h>
#include <lcpainter.h>
#include "cad/geometry/geoarea.h"

GradientBackground::GradientBackground(const QColor& topColor, const QColor& bottomColor) : LCVDrawItem(false), _topColor(topColor), _bottomColor(bottomColor) {
}

GradientBackground::~GradientBackground() {
}

void GradientBackground::draw(LcPainter* _painter, LcDrawOptions* options, const lc::geo::Area& updateRect) const {
    unsigned long patId = _painter->pattern_create_linear(0.0,  updateRect.minP().y(),  0.0, updateRect.maxP().y());

    _painter->pattern_add_color_stop_rgba(patId, 1., _bottomColor.redF(), _bottomColor.greenF(), _bottomColor.blueF(), _bottomColor.alphaF());
    _painter->pattern_add_color_stop_rgba(patId, 0., _topColor.redF(), _topColor.greenF(), _topColor.blueF(), _topColor.alphaF());
    _painter->rectangle(updateRect.minP().x(), updateRect.minP().y(), updateRect.width(), updateRect.height());
    _painter->set_source(patId);
    _painter->fill();
    _painter->pattern_destroy(patId);
}
