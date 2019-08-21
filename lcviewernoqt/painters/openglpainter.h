#pragma once

#include "lcpainter.h"
#include "renderer.h"
#include "gl_pack.h"
#include "cacher.h"
#include "cacherpainter.h"
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cad/geometry/geoarea.h>
#include <map>
#include <cad/meta/color.h>

#include <QOpenGLWidget>
using namespace lc::viewer;
using namespace lc::viewer::opengl;

#define PI 3.14159265
#define CURVE_POINTS 1000

         class LcOpenGLPainter : public LcPainter 
        {
            float _pen_x=0,_pen_y=0; //pen coordinates
          
            double _device_width=0.0;
            double _device_height=0.0;
            
            Renderer _renderer;
            LcPainter* _cacher_painter;

            public:
            LcOpenGLPainter(unsigned int width, unsigned int height);

                 void new_device_size(unsigned int width, unsigned int height);

                 void create_resources();

                 void new_path();

                 void close_path();

                 void new_sub_path();

                 void clear(double r, double g, double b);

                 void clear(double r, double g, double b, double a);

                 void move_to(double x, double y);

                 void line_to(double x, double y);

                 void lineWidthCompensation(double lwc); // When set, we add this to the current linewidth, perhaps we should find a better method

                 void line_width(double lineWidth);

                 double scale();

                 void scale(double s);

                 void rotate(double r);

                 void arc(double x, double y, double r, double start, double end);

                 void arcNegative(double x, double y, double r, double start, double end);

                 void circle(double x, double y, double r);

                 void ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra);

                 void rectangle(double x1, double y1, double w, double h);

                 void stroke();

                 void source_rgb(double r, double g, double b);

                 void source_rgba(double r, double g, double b, double a);

                 void translate(double x, double y);

                 void user_to_device(double* x, double* y);

                 void device_to_user(double* x, double* y);

                 void user_to_device_distance(double* dx, double* dy);

                 void device_to_user_distance(double* dx, double* dy);

                 void font_size(double size, bool deviceCoords);

                 void select_font_face(const char* text_val);

                 void text(const char* text_val);

                 TextExtends text_extends(const char* text_val);

                 void quadratic_curve_to(double x1, double y1, double x2, double y2);

                 void curve_to(double x1, double y1, double x2, double y2, double x3, double y3);

                 void save();

                 void restore();

                 long pattern_create_linear(double x1, double y1, double x2, double y2);

                 void pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a);

                 void set_pattern_source(long pat);

                 void pattern_destroy(long pat);

                 void fill();

                 void point(double x, double y, double size, bool deviceCoords);

                 void reset_transformations();

                 unsigned char* data();

                 void set_dash(const double* dashes, const int num_dashes, double offset, bool scaled);

                 void dash_destroy();
                 
                 long image_create(const std::string& file);

                 void image_destroy(long image);

                 void image(long image, double uvx, double vy, double vvx, double vvy, double x, double y);

                 void disable_antialias();

                 void enable_antialias();

                 void getTranslate(double* x, double* y);

                 void startcaching();

                 void finishcaching(unsigned long id);

                 LcPainter* getCacherpainter();

                 bool isEntityCached(unsigned long id);

                 void renderEntityCached(unsigned long id);

                 void deleteEntityCached(unsigned long id);


        };
  