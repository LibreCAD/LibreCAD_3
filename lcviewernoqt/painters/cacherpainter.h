#pragma once

#include "lcpainter.h"
#include "gl_pack.h"
#include "cacher.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <cad/geometry/geoarea.h>
#include <map>
#include <cad/meta/color.h>

using namespace lc::viewer;
using namespace lc::viewer::opengl;

#define PI 3.14159265
#define CURVE_POINTS 1000

class LcCacherPainter : public LcPainter 
{
  float _pen_x=0,_pen_y=0; //pen coordinates
     
  public:
  Cacher _cacher;
       
  LcCacherPainter();
  void create_resources() override;
  void new_device_size(unsigned int width, unsigned int height) override;

  void clear(double r, double g, double b) override;
  void clear(double r, double g, double b, double a) override;
  void new_path() override;
  void close_path() override;
  void new_sub_path() override;

  void move_to(double x, double y) override;
  void line_to(double x, double y) override;
  void lineWidthCompensation(double lwc) override; // When set, we add this to the current linewidth, perhaps we should find a better method
  void line_width(double lineWidth) override;

  void circle(double x, double y, double r) override;
  void ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra) override;
  void rectangle(double x1, double y1, double w, double h) override;
  void arc(double x, double y, double r, double start, double end) override;
  void arcNegative(double x, double y, double r, double start, double end) override;

  void stroke() override;
  void source_rgb(double r, double g, double b) override;
  void source_rgba(double r, double g, double b, double a) override;

  void scale(double s) override;
  double scale() override;
  void rotate(double r) override;
  void translate(double x, double y) override;
  void getTranslate(double* x, double* y) override;
  void save() override;
  void restore() override;
  
  void user_to_device_distance(double* dx, double* dy) override;
  void device_to_user_distance(double* dx, double* dy) override;
  void user_to_device(double* x, double* y) override;
  void device_to_user(double* x, double* y) override;

  void font_size(double size, bool deviceCoords) override;
  void select_font_face(const char* text_val) override;
  void text(const char* text_val) override;
  TextExtends text_extends(const char* text_val) override;

  void quadratic_curve_to(double x1, double y1, double x2, double y2) override;
  void curve_to(double x1, double y1, double x2, double y2, double x3, double y3) override;
  
  long pattern_create_linear(double x1, double y1, double x2, double y2) override;
  void pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a) override;
  void set_pattern_source(long pat) override;
  void pattern_destroy(long pat) override;
  void fill() override;
  void point(double x, double y, double size, bool deviceCoords) override;
  void reset_transformations() override;
  unsigned char* data() override;
  void set_dash(const double* dashes, const int num_dashes, double offset, bool scaled) override;
  void dash_destroy() override;
  
  long image_create(const std::string& file) override;
  void image_destroy(long image) override;
  void image(long image, double uvx, double vy, double vvx, double vvy, double x, double y) override;  void disable_antialias() override;
  void enable_antialias() override;
  
  bool isCachingEnabled() override;
  LcPainter* getCacherpainter() override;
  bool isEntityCached(unsigned long id) override;
  void renderEntityCached(unsigned long id) override;
  void startcaching() override;
  void finishcaching(unsigned long id) override;
  void deleteEntityCached(unsigned long id) override;
};