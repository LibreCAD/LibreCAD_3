#include "lccairopainter.h"

#include <cairo.h>
#include <math.h>
#include <array>

#include <QDebug>
#include <valarray>

LcCairoPainter::LcCairoPainter(cairo_surface_t* surface, cairo_t* cr) : LcPainter(), _surface(surface), _cr(cr) {
    _constantLineWidth = true;
    _lineWidth = 1.;
    _patternMapNum = 1;
    _lineWidthCompensation = 0.;
}

LcCairoPainter::~LcCairoPainter() {
    for (auto& item : _patternMap) {
        cairo_pattern_destroy(item.second);
    }

    if (_cr != nullptr) {
        cairo_destroy(_cr);
    }

    if (_surface != nullptr) {
        cairo_surface_destroy(_surface);
    }
}

#ifndef CAIRO_ANTIALIAS_GOOD
#define CAIRO_ANTIALIAS_GOOD    CAIRO_ANTIALIAS_SUBPIXEL
#endif

LcCairoPainter* LcCairoPainter::createImagePainter(unsigned char* data , int width, int height) {
    cairo_surface_t* surface;
    cairo_t* cr;
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32,  width,  height, stride);
    cr = cairo_create(surface);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_tolerance(cr, 0.25);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_GOOD);
    return new LcCairoPainter(surface, cr);
}

void LcCairoPainter::lineWidthCompensation(double lwc) {
    _lineWidthCompensation = lwc;
}

void LcCairoPainter::clear(double r, double g, double b)  {
    cairo_save(_cr);
    cairo_set_source_rgb(_cr, r, g, b);
    cairo_set_operator(_cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(_cr);
    cairo_restore(_cr);
}

void LcCairoPainter::clear(double r, double g, double b, double a)  {
    cairo_save(_cr);
    cairo_set_source_rgba(_cr, r, g, b, a);
    cairo_set_operator(_cr, CAIRO_OPERATOR_SOURCE);
    // We can consider using CAIRO_OPERATOR_CLEAR and not set a source at all?
    cairo_paint(_cr);
    cairo_restore(_cr);
}

void LcCairoPainter::move_to(double x, double y)  {
    cairo_move_to(_cr, x, y);
}

void LcCairoPainter::line_to(double x, double y)  {
    cairo_line_to(_cr, x, y);
}

void LcCairoPainter::arc(double x, double y, double r, double start, double end)  {
    cairo_arc(_cr, x, y, r, start, end);
}

void LcCairoPainter::circle(double x, double y, double r)  {
    cairo_arc(_cr, x, y, r, 0, 2 * M_PI);
}

void LcCairoPainter::ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra) {
    double cosrotangle = std::cos(ra);
    double sinrotangle = std::sin(ra);
    cairo_matrix_t transformmatrix;
    cairo_matrix_init(&transformmatrix, rx * cosrotangle, rx * sinrotangle, -ry * sinrotangle, ry * cosrotangle, cx, cy);
    cairo_save(_cr);
    cairo_transform(_cr, &transformmatrix);
    cairo_arc(_cr, 0, 0, 1, sa, ea);
    cairo_restore(_cr);
}

void LcCairoPainter::rectangle(double x1, double y1, double w, double y)  {
    cairo_rectangle(_cr, x1, y1, w, y);
}

void LcCairoPainter::stroke()  {
    cairo_stroke(_cr);
}

void LcCairoPainter::line_width(double  width) {
    if (_constantLineWidth) {
        cairo_set_line_width(_cr, (1. + _lineWidthCompensation) / scale());
    } else {
        cairo_set_line_width(_cr, width + _lineWidthCompensation);
    }

    _lineWidth = width;
}

double LcCairoPainter::scale() {
    cairo_matrix_t matrix;
    cairo_get_matrix(_cr, &matrix);
    return matrix.yy;
}

void LcCairoPainter::scale(double s) {
    cairo_scale(_cr, s, s);
    line_width(_lineWidth);
}

void LcCairoPainter::source_rgb(double r, double g, double b) {
    cairo_set_source_rgb(_cr, r, g, b);
}

void LcCairoPainter::source_rgba(double r, double g, double b, double a) {
    cairo_set_source_rgba(_cr, r, g, b, a);
}

void LcCairoPainter::translate(double x, double y) {
    cairo_translate(_cr, x, y);
}

void LcCairoPainter::user_to_device(double* x, double* y) {
    cairo_user_to_device(_cr, x, y);
}

void LcCairoPainter::device_to_user(double* x, double* y) {
    cairo_device_to_user(_cr, x, y);
}

void LcCairoPainter::user_to_device_distance(double* dx, double* dy) {
    cairo_user_to_device_distance(_cr, dx, dy);
}

void LcCairoPainter::device_to_user_distance(double* dx, double* dy) {
    cairo_device_to_user_distance(_cr, dx, dy);
}

void LcCairoPainter::save() {
    cairo_save(_cr);
}
void LcCairoPainter::restore() {
    cairo_restore(_cr);
}


unsigned long LcCairoPainter::pattern_create_linear(double x1, double y1, double x2, double y2) {
    _patternMap[++_patternMapNum] = cairo_pattern_create_linear(x1, y1, x2, y2);
    return _patternMapNum;
}

void LcCairoPainter::pattern_add_color_stop_rgba(unsigned long pat, double offset, double r, double g, double b, double a) {
    cairo_pattern_add_color_stop_rgba(_patternMap[pat], offset, r, g, b , a);
}

void LcCairoPainter::set_source(unsigned long pat) {
    cairo_set_source(_cr, _patternMap[pat]);
}

void LcCairoPainter::pattern_destroy(unsigned long pat) {
    cairo_pattern_destroy(_patternMap[pat]);
    _patternMap.erase(pat);
}

void LcCairoPainter::fill() {
    cairo_fill(_cr);
}

void LcCairoPainter::disable_antialias() {
    cairo_set_antialias(_cr, CAIRO_ANTIALIAS_NONE);
}

void LcCairoPainter::enable_antialias() {
    cairo_set_antialias(_cr, CAIRO_ANTIALIAS_GOOD);
}

void LcCairoPainter::reset_transformations() {
    cairo_identity_matrix(_cr);
}

unsigned char* LcCairoPainter::data() {
    return cairo_image_surface_get_data(_surface);
}

void LcCairoPainter::set_dash(const double *dashes, const int num_dashes, double offset, bool scaled) {
    if (scaled) {
        double scaledDashes[num_dashes];
        memcpy ( &scaledDashes, dashes, num_dashes * sizeof(double) );

        double _scale = scale();
        for (unsigned short c = 0; c < num_dashes; c++ ) {
            scaledDashes[c] = scaledDashes[c] / _scale;
        }

        cairo_set_dash(_cr, scaledDashes, num_dashes, offset);
    } else {
        cairo_set_dash(_cr, dashes, num_dashes, offset);
    }
}

