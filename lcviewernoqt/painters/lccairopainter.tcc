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
#include <mutex>
#include <cad/geometry/geocoordinate.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <functional>

#ifndef CAIRO_ANTIALIAS_GOOD
#define CAIRO_ANTIALIAS_GOOD    CAIRO_ANTIALIAS_SUBPIXEL
#endif

namespace CairoPainter {
    enum class backend : short {
        Image, PDF, SVG
    };
}

using namespace lc::viewer;


/**
 * A persistent store of data that can/must be shared between painters to reduce some overheads
 *
 * TODO: Since this is a persistence store I am wondering if this might 'leak',
 * eg, create more memory when the calling application doesn't properly destroy
 * cache images and patterns. Perhaps we can setup the cache such that
 * we can purge old entries automatically based on access time, not accessed for
 * 15 minutes we purge the item automatically.
 *
 */
class CairoStore {
    private:
        template<class T>
        struct Cairo_surface_store {
            Cairo_surface_store(const std::string& _name, T _surface) : name(_name), surface(_surface) {}

            const std::string name;
            const T surface;
        };

    public:
        CairoStore() : _patternMapNum(0), _imageMapNum(0) {
            _create_image_map = getImageMap();
        }

        virtual ~CairoStore() {
            for (auto& item : _patternMap) {
                cairo_pattern_destroy(item.second);
            }

            for (auto& item : _imageMap) {
                cairo_surface_destroy(item.second.surface);
            }
        }

        /**
         * Create a map of image surfaces from filenames
         * TODO: Add more images types like jpg, bmp etc...
         */
        static std::map<std::string, std::function<cairo_surface_t*(const std::string&)>> getImageMap() {
            std::map<std::string, std::function<cairo_surface_t*(const std::string&)>> create_image_map;
            create_image_map.insert(
                    std::make_pair("png", [](std::string f) {return cairo_image_surface_create_from_png(f.c_str());}));
            return create_image_map;
        }


        /**
         * Load image into a cairo surface
         * return's -1 if the surface wasn't loaded
         * TODO: Optimise Ensure we only load a image once
         */
        const long image_create(const std::string& file) {
            std::lock_guard<std::mutex> lck(_imageMapMutex);

            // test if we have this image already loaded
            for (auto i : _imageMap) {
                if (i.second.name == file) {
                    return i.first;
                }
            }

            // TODO: Support load files from URL
            GError* gdkError = NULL;
            GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(file.c_str(), &gdkError);

            if (!pixbuf) {
                std::cout << gdkError->message << std::endl;
                g_error_free(gdkError);

                return -1;
            } else {
                cairo_surface_t* image_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                                            gdk_pixbuf_get_width(pixbuf),
                                                                            gdk_pixbuf_get_height(pixbuf)
                );

                cairo_t* context = cairo_create(image_surface);
                gdk_cairo_set_source_pixbuf(context, pixbuf, 0, 0);
                cairo_paint(context);

                _imageMapNum++;
                _imageMap.emplace(_imageMapNum, Cairo_surface_store<cairo_surface_t*>(file, image_surface));

                cairo_destroy(context);
                g_object_unref(pixbuf);
                return _imageMapNum;
            }
        };

        void image_destroy(long image) {
            std::lock_guard<std::mutex> lck(_imageMapMutex);
            auto iLoc = _imageMap.find(image);
            if (iLoc != _imageMap.end()) {
                cairo_surface_destroy(_imageMap.at(image).surface);
                _imageMap.erase(iLoc);
            };
        }

        const long pattern_create_linear(double x1, double y1, double x2, double y2) {
            std::lock_guard<std::mutex> lck(_patternMapMutex);
            _patternMap[++_patternMapNum] = cairo_pattern_create_linear(x1, -y1, x2, -y2);
            return _patternMapNum;
        }

        void pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a) {
            cairo_pattern_add_color_stop_rgba(_patternMap.at(pat), offset, r, g, b, a);
        }

        void pattern_destroy(long pat) {
            std::lock_guard<std::mutex> lck(_patternMapMutex);
            cairo_pattern_destroy(_patternMap[pat]);
            _patternMap.erase(pat);
        }

        cairo_pattern_t* pattern(long pat) {
            return _patternMap.at(pat);
        }

        cairo_surface_t* image(long pat) {
            return _imageMap.at(pat).surface;
        }

    private:
        // List of tools to create image map's from different file types
        std::map<std::string, std::function<cairo_surface_t*(const std::string&)>> _create_image_map;

        // List of patterns
        std::map<long, cairo_pattern_t*> _patternMap;
        long _patternMapNum;
        std::mutex _patternMapMutex;

        // List of images
        std::map<long, Cairo_surface_store<cairo_surface_t*>> _imageMap;
        long _imageMapNum;
        std::mutex _imageMapMutex;
};

// Store to store long living dataset that are time consuming to generate
// TODO: Re-think this, I really don't like the idea of such global variable here
static CairoStore _store;


template<CairoPainter::backend T>
class LcCairoPainter : public LcPainter {
public:
    LcCairoPainter(cairo_surface_t *surface, cairo_t *cr) : _surface(surface), _cr(cr), _lineWidth(1.),
                                                            _constantLineWidth(true), _lineWidthCompensation(0.) {

    };

    template<CairoPainter::backend T_ = T>
    LcCairoPainter(typename std::enable_if<T_ == CairoPainter::backend::Image, unsigned char>::type *data,
                   unsigned int width, unsigned int height) : _constantLineWidth(true), _lineWidth(1.), _lineWidthCompensation(0.) {
        int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
        _surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32, width, height, stride);
        _cr = cairo_create(_surface);
        // Should we make a setting for this? Question, do other 2D painter support this sort of thing?
        cairo_set_fill_rule(_cr, CAIRO_FILL_RULE_WINDING);
//            cairo_set_fill_rule(_cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_set_operator(_cr, CAIRO_OPERATOR_OVER);
        cairo_set_tolerance(_cr, 0.5);
        cairo_set_antialias(_cr, CAIRO_ANTIALIAS_GOOD);

        reset_transformations();
    }

    /**
    * @brief backend constructor for PDF, SVG - join stream if needed
    *
    * @param width, height, cairo_status_t (void * closure, const uchar *data, unsigned int length)
    * @see http://cairographics.org/manual/cairo-PNG-Support.html#cairo-write-func-t
    */
    using cairo_stream_func = cairo_status_t(void *closure, const unsigned char *data, unsigned int length);
    LcCairoPainter(double width, double height, cairo_stream_func *f_) : _constantLineWidth(true), _lineWidth(1.), _lineWidthCompensation(0.) {

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

        reset_transformations();
    }

    ~LcCairoPainter() {


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
    void new_sub_path() {
        cairo_new_sub_path(_cr);
    }

    void new_path() {
        cairo_new_path(_cr);
    }

    void close_path() {
        cairo_close_path(_cr);
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
            // TODO: Not sure if we really require _lineWidthCompensation here
            cairo_set_line_width(_cr, ((width + _lineWidthCompensation) / scale()));
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

    /**
     * @brief Draw an arc clockwise
     */
    void arc(double x, double y, double r, double start, double end) {
        cairo_arc(_cr, x, -y, r, 2. * M_PI - start, 2. * M_PI - end);
    }

    void arcNegative(double x, double y, double r, double start, double end) {
        cairo_arc_negative(_cr, x, -y, r, 2. * M_PI - start, 2. * M_PI - end);
    }

    void circle(double x, double y, double r) {
        cairo_arc(_cr, x, -y, r, 0, 2. * M_PI);
        std::cout<<"painter Circle"<<x<<","<<y<<std::endl;
    }

    void ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra = 0) {
        if(rx == 0) {
            rx = 0.1;
        }
        if(ry == 0) {
            ry = 0.1;
        }

        cairo_save(_cr);
        cairo_translate(_cr, cx, - cy);
        cairo_rotate(_cr, - ra);
        cairo_scale(_cr, rx, ry);
        if (ea == sa)
            cairo_arc(_cr, 0, 0, 1, 0, 2*M_PI);
        else
            cairo_arc(_cr, 0, 0, 1, 2*M_PI - ea, 2*M_PI - sa);
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

    void font_size(double size, bool deviceCoords) {
        if (deviceCoords) {
            cairo_set_font_size(_cr, size / scale() );
        } else {
            cairo_set_font_size(_cr, size);
        }
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

    void quadratic_curve_to(double x1, double y1, double x2, double y2) {
        // From : https://lists.cairographics.org/archives/cairo/2009-October/018351.html
        // http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/bezier-elev.html

        // From https://github.com/mono/moon/blob/master/src/moon-path.cpp#L285
        y2 = -y2;
        y1 = -y1;
        double x0, y0;
        double x3 = x2;
        double y3 = y2;
        cairo_get_current_point(_cr, &x0, &y0);
        x2 = x1 + (x2 - x1) / 3.;
        y2 = y1 + (y2 - y1) / 3.;
        x1 = x0 + 2. * (x1 - x0) / 3.;
        y1 = y0 + 2. * (y1 - y0) / 3.;
        cairo_curve_to(_cr, x1, y1, x2, y2, x3, y3);
    }

    void curve_to(double x1, double y1, double x2, double y2, double x3, double y3) {
        cairo_curve_to(_cr, x1, -y1, x2, -y2, x3, -y3);
    }

    void save() {
        cairo_save(_cr);
    }

    void restore() {
        cairo_restore(_cr);
    }

    long pattern_create_linear(double x1, double y1, double x2, double y2) {
        return _store.pattern_create_linear(x1, y1, x2, y2);
    }

    void pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a) {
        _store.pattern_add_color_stop_rgba(pat, offset, r, g, b, a);
    }

    void set_pattern_source(long pat) {
        cairo_set_source(_cr, _store.pattern(pat));
    }

    void pattern_destroy(long pat) {
        _store.pattern_destroy(pat);
    }

    void fill() {
        cairo_fill(_cr);
    }
    
   //##############################################################
    //############# Blank Functions to compatible ##################
    //#################### with OpenGL  ############################
    //##############################################################
    
    void create_resources(){ 
    }                            

    void new_device_size(unsigned int width, unsigned int height){
    }

    bool isCachingEnabled()
    {
      return false;
    }

    void startcaching(){
    }

    void finishcaching(unsigned long id){     
    }

    LcPainter* getCacherpainter(){
    }

    bool isEntityCached(unsigned long id){
    }

    void renderEntityCached(unsigned long id){
    }

    void deleteEntityCached(unsigned long id){                
    }

    void dash_destroy(){
    }

    std::vector<std::string> getFontList() const {
        return std::vector<std::string>();
    }

    void addFontsFromPath(const std::vector<std::string>& paths) {
    }


   //##############################################################
    //##############################################################
    //##############################################################
    //##############################################################
    
    
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

    /**
     * Warning, if scaled is set to true then this takes additional processing
     * TODO: can we optmise the calculation when scale is set to true?
     */
    void set_dash(const double *dashes, const int num_dashes, double offset, bool scaled) {
        if (scaled) {
            auto scaledDashes = new double[num_dashes];
            memcpy(scaledDashes, dashes, num_dashes * sizeof(double));

            double _scale = scale();

            for (short c = 0; c < num_dashes; c++) {
                scaledDashes[c] = scaledDashes[c] / _scale;
            }

            cairo_set_dash(_cr, scaledDashes, num_dashes, offset);
            delete scaledDashes;
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

    /**
     * Loda image into a cairo surface
     * return's -1 if the surface wasn't loaded
     * TODO: Optimise Ensure we only load a image once
     */
    virtual long image_create(const std::string &file) {
        return _store.image_create(file);
    };

    virtual void image_destroy(long image) {
        _store.image_destroy(image);

    }

    virtual void image(long image, double uvx, double uvy, double vvx, double vvy, double x, double y) {
        auto i = _store.image(image);
        if (i != nullptr) {
            auto h = cairo_image_surface_get_height(i);
            auto a = lc::geo::Coordinate(uvx, uvy).angle();
            //auto w = cairo_image_surface_get_width(i);

            cairo_save(_cr);
            cairo_matrix_t transformmatrix;
            cairo_matrix_init(&transformmatrix, uvx, uvy, vvx, vvy, 0., 0.);

            // Apparently for the DXF I need to invert the matrix
            cairo_matrix_invert(&transformmatrix);

            /** START: This code might bite me in my back, I am just not sure how that's gonna happen
             * Most lickly when you are trying to test skew, but at this time I am not 100% sure about that
             * LibreCAD 2 and draftsight doesn't provide a test for that but considering the uv vv values
             * this might be possible. I just didn't know how to do it without this rotate to correctly
             * show the image */
            cairo_matrix_rotate(&transformmatrix, a);
            cairo_matrix_translate(&transformmatrix, 0., h);
            cairo_matrix_rotate(&transformmatrix, a);
            /* END: This code might bite me in my back....... */

            cairo_matrix_translate(&transformmatrix, -x, y);
            auto pattern = cairo_pattern_create_for_surface(i);
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE);
            cairo_pattern_set_matrix(pattern, &transformmatrix);
            cairo_set_source(_cr, pattern);
            cairo_paint(_cr);
            cairo_pattern_destroy(pattern);
            cairo_restore(_cr);
        } else {
            // Perhaps we can render a file not find text?
        }
    }

    // TODO: move this somewhere more universal but without lib dependencies
    static std::string tolower(std::string data) {
        std::transform(data.begin(), data.end(), data.begin(), ::tolower);
        return data;
    }

private:
    cairo_surface_t *_surface;
    cairo_t *_cr;

    // When set to true, the linewidth will be constant, eg, it won't scale with the scale factor
    bool _constantLineWidth;

    // Current Line width
    double _lineWidth;

    // When set to > 0. it add's a bit of linewidth so extra thin lines will appear a bit better
    double _lineWidthCompensation = 0.;

};
