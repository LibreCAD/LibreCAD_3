#include "gradientbackground.h"

#include <QDebug>
#include <QRectF>
#include <QGraphicsView>
#include <QPainter>

#include <cairo.h>

GradientBackground::GradientBackground(const QColor& topColor, const QColor& bottomColor) : _topColor(topColor), _bottomColor(bottomColor) {
}

GradientBackground::~GradientBackground() {
    qDebug() << "GradientBackground destroyed";
}

void GradientBackground::draw(cairo_t* cr, const QRectF& rect) {

    // BAckground
    cairo_pattern_t* pat;
    pat = cairo_pattern_create_linear(0.0, 0.0,  0.0, rect.height());
    cairo_pattern_add_color_stop_rgba(pat, 1, _bottomColor.redF(), _bottomColor.greenF(), _bottomColor.blueF(), _bottomColor.alphaF());
    cairo_pattern_add_color_stop_rgba(pat, 0, _topColor.redF(), _topColor.greenF(), _topColor.blueF(), _topColor.alphaF());
    cairo_rectangle(cr, 0, 0, rect.width(), rect.height());
    cairo_set_source(cr, pat);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
}
