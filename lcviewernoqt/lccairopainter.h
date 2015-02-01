#pragma once

#include <cairo.h>
#include "lcpainter.h"
#include <map>
#include <pango/pangocairo.h>
class LcCairoPainter : public LcPainter {
    public:
        LcCairoPainter(cairo_surface_t* surface, cairo_t* cr);
        virtual ~LcCairoPainter();
        static LcCairoPainter* createImagePainter(unsigned char* data , int width, int height);
        static LcCairoPainter* createPainter(int width, int height);
        void writePNG(std::string fileName);

    public:
        // TODO move code from implementation to here to actually make inlyning work
        virtual void new_path();
        virtual void clear(double r, double g, double b) ;
        virtual void clear(double r, double g, double b, double a) ;
        virtual void move_to(double x, double y) ;
        virtual void line_to(double x, double y) ;
        virtual void lineWidthCompensation(double lwc);
        virtual void line_width(double w) ;
        virtual double scale() ;
        virtual void rotate(double r);
        virtual void scale(double s) ;
        virtual void arc(double x, double y, double r, double start, double end) ;
        virtual void arcNegative(double x, double y, double r, double start, double end) ;
        virtual void circle(double x, double y, double r) ;
        virtual void ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra = 0);
        virtual void rectangle(double x1, double y1, double w, double h);
        virtual void stroke() ;
        virtual void source_rgb(double r, double g, double b);
        virtual void source_rgba(double r, double g, double b, double a);
        virtual void translate(double x, double y);
        virtual void user_to_device(double* x, double* y);
        virtual void device_to_user(double* x, double* y);
        virtual void user_to_device_distance(double* dx, double* dy);
        virtual void device_to_user_distance(double* dx, double* dy);
        virtual void select_font_face(const char* text_val);
        virtual void font_size(double size);
        virtual void text(const char* text_val);
        virtual TextExtends text_extends(const char* text_val);
        virtual void save();
        virtual void restore();
        virtual unsigned long pattern_create_linear(double x1, double y1, double x2, double y2);
        virtual void pattern_add_color_stop_rgba(unsigned long pat, double offset, double r, double g, double b, double a);
        virtual void set_source(unsigned long pat);
        virtual void pattern_destroy(unsigned long pat);
        virtual void fill();
        virtual void point(double x, double y, double size, bool deviceCoords);
        virtual void reset_transformations();
        virtual unsigned char* data();
        // virtual void arrow(double startX, double startY, double endX, double endY, const CapType& startCap, const CapType& endCap);
        virtual void set_dash(const double* dashes, const int num_dashes, double offset, bool scaled);

        // The functions below might get changed in future
        virtual void disable_antialias();
        virtual void enable_antialias();

        virtual void getTranslate(double* x, double* y);


    private:
        cairo_surface_t* _surface;
        cairo_t* _cr;

        // When set to true, the linewidth will be constant, eg, it won't scale with the scale factor
        bool _constantLineWidth;

        // Current Line width
        double _lineWidth;

        // List of patterns
        std::map<unsigned long, cairo_pattern_t*> _patternMap;
        unsigned long _patternMapNum;

        double _lineWidthCompensation;
};
