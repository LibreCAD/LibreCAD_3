#pragma once

#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>
#include "lcpainter.h"
#include <math.h>
#include <array>
#include <string.h>
#include <valarray>
#include <iostream>
#include <map>
#include <pango/pangocairo.h>
//class LcCairoPainter;
#ifndef CAIRO_ANTIALIAS_GOOD
#define CAIRO_ANTIALIAS_GOOD    CAIRO_ANTIALIAS_SUBPIXEL
#endif

namespace CairoPainter {
    enum class backend : short {
        Image, PDF, SVG
    };
};


template<CairoPainter::backend T>
class LcCairoPainter : public LcPainter {
    public:
        LcCairoPainter(cairo_surface_t *surface, cairo_t *cr) : _surface(surface), _cr(cr) {};

        template<CairoPainter::backend T_ = T>
        LcCairoPainter( typename std::enable_if<T_ == CairoPainter::backend::Image, unsigned char>::type *data, 
                        uint width, uint height ) {
            int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
            _surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32,  width,  height, stride);
            _cr = cairo_create(_surface);
            cairo_set_fill_rule(_cr, CAIRO_FILL_RULE_EVEN_ODD);
            cairo_set_operator(_cr, CAIRO_OPERATOR_OVER);
            cairo_set_tolerance(_cr, 0.5);
            cairo_set_antialias(_cr, CAIRO_ANTIALIAS_GOOD);
        }
        
        /**
        * @brief backend constructor for PDF, SVG - join stream if needed
        * 
        * @param width, height, cairo_status_t (void * closure, const uchar *data, uint length)
        * @see http://cairographics.org/manual/cairo-PNG-Support.html#cairo-write-func-t
        */
        using cairo_stream_func = cairo_status_t (void * closure, const unsigned char *data, uint length);
        LcCairoPainter(double width, double height, cairo_stream_func * f_) {
            
            switch (T) {
                case CairoPainter::backend::PDF: {
                    /* _surface = cairo_pdf_surface_create("fastforward.pdf", width, height); */
                    _surface = cairo_pdf_surface_create_for_stream(f_, nullptr, width, height);
                }
                break;
            
                case CairoPainter::backend::SVG: {
                    _surface = cairo_svg_surface_create_for_stream(f_, nullptr, width, height);
                }
                break;
            }
            _cr = cairo_create(_surface);
            cairo_set_fill_rule(_cr, CAIRO_FILL_RULE_EVEN_ODD);
            cairo_set_operator(_cr, CAIRO_OPERATOR_OVER);
            cairo_set_tolerance(_cr, 0.5);
            cairo_set_antialias(_cr, CAIRO_ANTIALIAS_GOOD);
            
        }

        ~LcCairoPainter() {
            for (auto & item : _patternMap) {
                cairo_pattern_destroy(item.second);
            }

            if (_cr != nullptr) {
                cairo_destroy(_cr);
            }

            if (_surface != nullptr) {
                cairo_surface_destroy(_surface);
            }
        }


        //template<CairoPainter::backend T_>
        //static LcCairoPainter<T>* createImagePainter(unsigned char* data , int width, int height);

        //template<CairoPainter::backend T_>
        //static LcCairoPainter<T>* createPainter(int width, int height);
        void writePNG(std::string filename) {
            cairo_surface_write_to_png(_surface, filename.c_str());
        }

    public:
        // TODO move code from implementation to here to actually make inlyning work
        void new_path() {
            cairo_new_path(_cr);
        }

        void clear(double r, double g, double b) {
            cairo_save(_cr);
            cairo_set_source_rgb(_cr, r, g, b);
            cairo_set_operator(_cr, CAIRO_OPERATOR_SOURCE);
            cairo_paint(_cr);
            cairo_restore(_cr);
        }

        void clear(double r, double g, double b, double a) {
            cairo_save(_cr);
            cairo_set_source_rgba(_cr, r, g, b, a);
            cairo_set_operator(_cr, CAIRO_OPERATOR_SOURCE);
            // We can consider using CAIRO_OPERATOR_CLEAR and not set a source at all?
            cairo_paint(_cr);
            cairo_restore(_cr);
        }

        void move_to(double x, double y) {
            cairo_move_to(_cr, x, -y);
        }

        void line_to(double x, double y) {
            cairo_line_to(_cr, x, -y);
        }

        void lineWidthCompensation(double lwc) {
            _lineWidthCompensation = lwc;
        };
        void line_width(double width) {
            if (_constantLineWidth) {
                cairo_set_line_width(_cr, (width + _lineWidthCompensation) / scale());
            } else {
                cairo_set_line_width(_cr, width + _lineWidthCompensation);
            }

            _lineWidth = width;
        }
        double scale() {
            cairo_matrix_t matrix;
            cairo_get_matrix(_cr, &matrix);
            return matrix.yy;
        }

        void rotate(double r) {
            cairo_rotate(_cr, r);
        }

        void scale(double s) {
            cairo_scale(_cr, s, s);
            line_width(_lineWidth);
        }

        void arc(double x, double y, double r, double start, double end) {
            cairo_arc(_cr, x, -y, r, 2. * M_PI - end, 2.* M_PI - start);
            /* Test to see if we can get end/start cap's on a path
            cairo_path_t *path;
            cairo_path_data_t *data;
            path = cairo_copy_path_flat (_cr);
            cairo_stroke (_cr);

            data = &path->data[0];
            cairo_arc(_cr, data[1].point.x, data[1].point.y, 3. / scale() , 0, 2 * M_PI);
            cairo_fill(_cr);

            data = &path->data[path->num_data - path->data[0].header.length];
            cairo_arc(_cr, data[1].point.x, data[1].point.y, 3. / scale() , 0, 2 * M_PI);
            cairo_fill(_cr);

            cairo_path_destroy (path); */
        }

        void arcNegative(double x, double y, double r, double start, double end) {
            cairo_arc_negative(_cr, x, -y, r, 2. * M_PI - end, 2.* M_PI - start);
//            cairo_arc_negative(_cr, x, -y, r, start, end);
        }

        void circle(double x, double y, double r) {
            cairo_arc(_cr, x, -y, r, 0, 2. * M_PI);
        }

        void ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra = 0) {
            double cosrotangle = std::cos(ra);
            double sinrotangle = std::sin(ra);
            cairo_matrix_t transformmatrix;
            cairo_matrix_init(&transformmatrix, rx * cosrotangle, rx * sinrotangle, ry * sinrotangle, -ry * cosrotangle, cx, -cy);
            cairo_save(_cr);
            cairo_transform(_cr, &transformmatrix);
            cairo_arc(_cr, 0, 0, 1, sa, ea);
            cairo_restore(_cr);
        }

        void rectangle(double x1, double y1, double w, double h) {
            cairo_rectangle(_cr, x1, -y1, w, -h);
        }

        void stroke() {
            cairo_stroke(_cr);
        }
        void source_rgb(double r, double g, double b) {
            cairo_set_source_rgb(_cr, r, g, b);
        }
        void source_rgba(double r, double g, double b, double a) {
            cairo_set_source_rgba(_cr, r, g, b, a);
        }
        void translate(double x, double y) {
            cairo_translate(_cr, x, y);
        }
        void user_to_device(double *x, double *y) {
            *y = -*y;
            cairo_user_to_device(_cr, x, y);
        }
        void device_to_user(double *x, double *y) {
            cairo_device_to_user(_cr, x, y);
            *y = -*y;
        }
        void user_to_device_distance(double *dx, double *dy) {
            *dy = -*dy;
            cairo_user_to_device_distance(_cr, dx, dy);
        }
        void device_to_user_distance(double *dx, double *dy) {
            cairo_device_to_user_distance(_cr, dx, dy);
            *dy = -*dy;
        }
        void select_font_face(const char *text_val) {
            cairo_select_font_face(_cr, text_val, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        }
        void font_size(double size) {
            cairo_set_font_size(_cr, size);
        }
        void text(const char *text_val) {
            cairo_show_text(_cr, text_val);
        }

        TextExtends text_extends(const char *text_val) {
            TextExtends te;

            cairo_text_extents_t extents;
            cairo_text_extents(_cr, text_val, &extents);

            te.height = extents.height;
            te.width = extents.width;
            te.x_advance = extents.x_advance;
            te.x_bearing = extents.x_bearing;
            te.y_advance = extents.y_advance;
            te.y_bearing = extents.y_bearing;

            return te;
        }
        void save() {
            cairo_save(_cr);
        }
        void restore() {
            cairo_restore(_cr);
        }
        unsigned long pattern_create_linear(double x1, double y1, double x2, double y2) {
            _patternMap[++_patternMapNum] = cairo_pattern_create_linear(x1, y1, x2, y2);
            return _patternMapNum;
        }
        void pattern_add_color_stop_rgba(unsigned long pat, double offset, double r, double g, double b, double a) {
            cairo_pattern_add_color_stop_rgba(_patternMap[pat], offset, r, g, b , a);
        }
        void set_source(unsigned long pat) {
            cairo_set_source(_cr, _patternMap[pat]);
        }
        void pattern_destroy(unsigned long pat) {
            cairo_pattern_destroy(_patternMap[pat]);
            _patternMap.erase(pat);
        }
        void fill() {
            cairo_fill(_cr);
        }
        void point(double x, double y, double size, bool deviceCoords) {
            if (deviceCoords) {
                cairo_arc(_cr, x, -y, size / scale(), 0, 2. * M_PI);
            } else {
                cairo_arc(_cr, x, -y, size, 0, 2 * M_PI);
            }

            cairo_fill(_cr);
        }
        void reset_transformations() {
            cairo_identity_matrix(_cr);
        }
        unsigned char *data() {
            return cairo_image_surface_get_data(_surface);
        }

        void set_dash(const double *dashes, const int num_dashes, double offset, bool scaled) {
            if (scaled) {
                double scaledDashes[num_dashes];
                memcpy(&scaledDashes, dashes, num_dashes * sizeof(double));

                double _scale = scale();

                for (short c = 0; c < num_dashes; c++) {
                    scaledDashes[c] = scaledDashes[c] / _scale;
                }

                cairo_set_dash(_cr, scaledDashes, num_dashes, offset);
            } else {
                cairo_set_dash(_cr, dashes, num_dashes, offset);
            }
        }


        void disable_antialias() {
            cairo_set_antialias(_cr, CAIRO_ANTIALIAS_NONE);
        }
        void enable_antialias() {
            cairo_set_antialias(_cr, CAIRO_ANTIALIAS_GOOD);
        }

        void getTranslate(double *x, double *y) {
            cairo_matrix_t matrix;
            cairo_get_matrix(_cr, &matrix);
            *x = matrix.x0;
            *y = matrix.y0;
        }


    private:
        cairo_surface_t *_surface;
        cairo_t *_cr;

// When set to true, the linewidth will be constant, eg, it won't scale with the scale factor
        bool _constantLineWidth = true;

// Current Line width
        double _lineWidth = 1.;

// List of patterns
        std::map<unsigned long, cairo_pattern_t *> _patternMap;
        unsigned long _patternMapNum = 1;

        double _lineWidthCompensation = 0.;
};
