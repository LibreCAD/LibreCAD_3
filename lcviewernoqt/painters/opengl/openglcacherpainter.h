#pragma once

#include "lcpainter.h"
#include "renderer.h"
#include "gl_pack.h"
#include "cacher.h"
#include "openglpainter.h"

using namespace lc::viewer;
using namespace lc::viewer::opengl;

#define PI 3.14159265
#define CURVE_POINTS 1000

class OpenglCacherPainter : public OpenglPainter
{
public:
  Cacher* _cacher;

  OpenglCacherPainter();

  double scale() override;
  void scale(double s) override;
  void rotate(double r) override;
  
  void stroke() override;
  
  void text(const char* text_val) override;
  TextExtends text_extends(const char* text_val) override;
  
  long pattern_create_linear(double x1, double y1, double x2, double y2) override;
  void pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a) override;
  void set_pattern_source(long pat) override;
  void pattern_destroy(long pat) override;
 
  void reset_transformations() override;
  unsigned char* data() override;

  void disable_antialias() override;
  void enable_antialias() override;
  
  long image_create(const std::string& file) override;
  void image_destroy(long image) override;
  void image(long image, double uvx, double vy, double vvx, double vvy, double x, double y) override;
  
  void getTranslate(double* x, double* y) override;
  
  bool isCachingEnabled() override;
  void startcaching() override;
  void finishcaching(unsigned long id) override;
  LcPainter* getCacherpainter() override;
  bool isEntityCached(unsigned long id) override;
  void renderEntityCached(unsigned long id) override;
  void deleteEntityCached(unsigned long id) override;

  //--------No Need----
  void new_device_size(unsigned int width, unsigned int height) override;
  void create_resources() override;
  void clear(double r, double g, double b) override;
  void clear(double r, double g, double b, double a) override;
  
  void source_rgb(double r, double g, double b) override;
  void source_rgba(double r, double g, double b, double a) override;

  void save() override;
  void restore() override;
  
  void translate(double x, double y) override;
  void user_to_device(double* x, double* y) override;
  void device_to_user(double* x, double* y) override;
  void user_to_device_distance(double* dx, double* dy) override;
  void device_to_user_distance(double* dx, double* dy) override;
};