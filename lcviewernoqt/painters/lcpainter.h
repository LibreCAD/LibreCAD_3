#pragma once

#include <string>
namespace lc {
    namespace viewer {

        typedef struct {
            double x_bearing;
            double y_bearing;
            double width;
            double height;
            double x_advance;
            double y_advance;
        } TextExtends;

        class LcPainter {
            public:
                virtual ~LcPainter() = default;

                virtual void create_resources() = 0;

                virtual void new_device_size(unsigned int width, unsigned int height) = 0;

                virtual void new_path() = 0;

                virtual void close_path() = 0;

                virtual void new_sub_path() = 0;

                virtual void clear(double r, double g, double b) = 0;

                virtual void clear(double r, double g, double b, double a) = 0;

                virtual void move_to(double x, double y) = 0;

                virtual void line_to(double x, double y) = 0;

                virtual void lineWidthCompensation(double lwc) = 0; // When set, we add this to the current linewidth, perhaps we should find a better method

                virtual void line_width(double lineWidth) = 0;

                virtual double scale() = 0;

                virtual void scale(double s) = 0;

                virtual void rotate(double r) = 0;

                virtual void arc(double x, double y, double r, double start, double end) = 0;

                virtual void arcNegative(double x, double y, double r, double start, double end) = 0;

                virtual void circle(double x, double y, double r) = 0;

                virtual void ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra = 0) = 0;

                virtual void rectangle(double x1, double y1, double w, double h) = 0;

                virtual void stroke() = 0;

                virtual void source_rgb(double r, double g, double b) = 0;

                virtual void source_rgba(double r, double g, double b, double a) = 0;

                virtual void translate(double x, double y) = 0;

                virtual void user_to_device(double* x, double* y) = 0;

                virtual void device_to_user(double* x, double* y) = 0;

                virtual void user_to_device_distance(double* dx, double* dy) = 0;

                virtual void device_to_user_distance(double* dx, double* dy) = 0;

                virtual void font_size(double size, bool deviceCoords) = 0;

                virtual void select_font_face(const char* text_val) = 0;

                virtual void text(const char* text_val) = 0;

                virtual TextExtends text_extends(const char* text_val) = 0;

                virtual void quadratic_curve_to(double x1, double y1, double x2, double y2) = 0;

                virtual void curve_to(double x1, double y1, double x2, double y2, double x3, double y3) = 0;

                virtual void save() = 0;

                virtual void restore() = 0;

                virtual long pattern_create_linear(double x1, double y1, double x2, double y2) = 0;

                virtual void pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a) = 0;

                virtual void set_pattern_source(long pat) = 0;

                virtual void pattern_destroy(long pat) = 0;

                virtual void fill() = 0;

                virtual void point(double x, double y, double size, bool deviceCoords) = 0;

                virtual void reset_transformations() = 0;

                virtual unsigned char* data() = 0;

                //        virtual inline void arrow(double startX, double startY, double endX, double endY, const CapType& startCap, const CapType& endCap) = 0;
                virtual void set_dash(const double* dashes, const int num_dashes, double offset, bool scaled) = 0;
                
                virtual void dash_destroy() = 0;
                
                virtual long image_create(const std::string& file) = 0;

                virtual void image_destroy(long image) = 0;

                virtual void image(long image, double uvx, double vy, double vvx, double vvy, double x, double y) = 0;

                // The functions below might get changed in future
                virtual void disable_antialias() = 0;

                virtual void enable_antialias() = 0;

                // We should consider returning a matrix?
                virtual void getTranslate(double* x, double* y) = 0;

                // functions for caching mechanism
                virtual bool isCachingEnabled() = 0;
                
                virtual void startcaching() = 0;

                virtual void finishcaching(unsigned long id) = 0;

                virtual LcPainter* getCacherpainter() = 0;

                virtual bool isEntityCached(unsigned long id) = 0;

                virtual void renderEntityCached(unsigned long id) = 0;

                virtual void deleteEntityCached(unsigned long id) = 0;

               
        };
    }
}
