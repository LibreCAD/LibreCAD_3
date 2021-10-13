#pragma once

#include <painters/lcpainter.h>
#include "manager.h"
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cad/geometry/geoarea.h>
#include <map>
#include <cmath>
#include <cstring>
#include <cad/meta/color.h>

using namespace lc::viewer;
using namespace lc::viewer::opengl;

#define PI 3.14159265
#define CURVE_POINTS 1000

class OpenglPainter : public LcPainter
{
    float _pen_x=0,_pen_y=0; //pen coordinates
    Manager* _manager=NULL;

public:
    OpenglPainter();
    void set_manager(Manager* manager);
    void move_to(double x, double y) override;
    void point(double x, double y, double size, bool deviceCoords) override;
    void line_to(double x, double y) override;
    void arc(double x, double y, double r, double start, double end) override;
    void arcNegative(double x, double y, double r, double start, double end) override;
    void circle(double x, double y, double r) override;
    void ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra) override;
    void rectangle(double x1, double y1, double w, double h) override;
    void quadratic_curve_to(double x1, double y1, double x2, double y2) override;
    void curve_to(double x1, double y1, double x2, double y2, double x3, double y3) override;

    void close_path() override;
    void fill() override;
    void new_path() override;
    void new_sub_path() override;

    void lineWidthCompensation(double lwc) override;
    void line_width(double lineWidth) override;

    void font_size(double size, bool deviceCoords) override;
    void select_font_face(const char* text_val, const char* font_type) override;

    void set_dash(const double* dashes, const int num_dashes, double offset, bool scaled) override;
    void dash_destroy() override;
};
