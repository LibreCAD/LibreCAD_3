#ifndef LCCAIROPAINTER_H
#define LCCAIROPAINTER_H

#include <cairo.h>
#include "lcpainter.h"
#include <map>
#include <pango/pangocairo.h>
#include "endcaps.h"
class LcCairoPainter : public LcPainter {
    public:
        LcCairoPainter(cairo_surface_t* surface, cairo_t* cr);
        virtual ~LcCairoPainter();
        static LcCairoPainter* createImagePainter(unsigned char* data , int width, int height);

    public:
        // TODO move code from implementation to here to actually make inlyning work
        virtual inline void clear(double r, double g, double b) ;
        virtual inline void clear(double r, double g, double b, double a) ;
        virtual inline void move_to(double x, double y) ;
        virtual inline void line_to(double x, double y) ;
        virtual inline void lineWidthCompensation(double lwc);
        virtual inline void line_width(double w) ;
        virtual inline double scale() ;
        virtual inline void scale(double s) ;
        virtual inline void arc(double x, double y, double r, double start, double end) ;
        virtual inline void circle(double x, double y, double r) ;
        virtual inline void ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra = 0);
        virtual inline void rectangle(double x1, double y1, double w, double h);
        virtual inline void stroke() ;
        virtual inline void source_rgb(double r, double g, double b);
        virtual inline void source_rgba(double r, double g, double b, double a);
        virtual inline void translate(double x, double y);
        virtual inline void user_to_device(double* x, double* y);
        virtual inline void device_to_user(double* x, double* y);
        virtual inline void user_to_device_distance(double* dx, double* dy);
        virtual inline void device_to_user_distance(double* dx, double* dy);
        virtual inline void text(double x, double y, const char* text_val, double angle, double height);
        virtual inline void save();
        virtual inline void restore();
        virtual inline unsigned long pattern_create_linear(double x1, double y1, double x2, double y2);
        virtual inline void pattern_add_color_stop_rgba(unsigned long pat, double offset, double r, double g, double b, double a);
        virtual inline void set_source(unsigned long pat);
        virtual inline void pattern_destroy(unsigned long pat);
        virtual inline void fill();
        virtual inline void point(double x, double y);
        virtual inline void reset_transformations();
        virtual inline unsigned char* data();
        virtual inline void arrow(double startX, double startY, double endX, double endY, const CapType& startCap, const CapType& endCap);
        virtual inline void set_dash(const double* dashes, const int num_dashes, double offset, bool scaled);

        // The functions below might get changed in future
        virtual inline void disable_antialias();
        virtual inline void enable_antialias();

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

#endif // LCCAIROPAINTER_H
