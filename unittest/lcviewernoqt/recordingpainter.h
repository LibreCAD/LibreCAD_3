#pragma once

// A painter that draws nothing and remembers where it was asked to.
//
// LcPainter has sixty-one pure virtuals and no implementation outside the GL
// and cairo backends, both of which need a device. That is why nothing under
// drawitems/ has ever had a test: the arithmetic could be extracted and
// checked, but whether draw() passes the right arithmetic the right arguments
// could not. Sixty-one empty bodies fix that for every draw item, not just
// the one this file arrived with.
//
// What it models is the transform stack, because that is what placement is:
// translate, rotate, save and restore compose into an affine matrix, and a
// text() call records where the current point lands in device space. Scale is
// modelled too, so a caller that scales is not silently ignored.
//
// text_extends is deterministic on purpose -- width is the character count
// times half the font size, height is the font size -- so a test can state
// the offsets it expects in closed form instead of reading them back.

#include <cstring>
#include <string>
#include <vector>

#include <painters/lcpainter.h>

namespace lc {
namespace viewer {
namespace testing {

/// A 2-D affine transform, row-major: [a c e; b d f].
struct Affine {
    double a{1.0}, b{0.0}, c{0.0}, d{1.0}, e{0.0}, f{0.0};

    Affine then(const Affine& outer) const {
        // outer * this, so applying `this` first and `outer` second.
        Affine out;
        out.a = outer.a * a + outer.c * b;
        out.b = outer.b * a + outer.d * b;
        out.c = outer.a * c + outer.c * d;
        out.d = outer.b * c + outer.d * d;
        out.e = outer.a * e + outer.c * f + outer.e;
        out.f = outer.b * e + outer.d * f + outer.f;
        return out;
    }

    void apply(double x, double y, double* outX, double* outY) const {
        *outX = a * x + c * y + e;
        *outY = b * x + d * y + f;
    }
};

/// One text() call, with the device-space point it was drawn from.
struct DrawnText {
    std::string text;
    double x{0.0};
    double y{0.0};
    double fontSize{0.0};
};

/// One stroked segment, in device space. Underlines and strikethroughs.
struct DrawnLine {
    double x1{0.0}, y1{0.0}, x2{0.0}, y2{0.0};
};

class RecordingPainter : public LcPainter {
public:
    std::vector<DrawnText> texts;
    std::vector<DrawnLine> lines;

    // -- the parts that are modelled ------------------------------------
    void save() override {
        _stack.push_back(_transform);
    }

    void restore() override {
        if (!_stack.empty()) {
            _transform = _stack.back();
            _stack.pop_back();
        }
    }

    void translate(double x, double y) override {
        Affine t;
        t.e = x;
        t.f = y;
        _transform = t.then(_transform);
    }

    void rotate(double r) override {
        Affine t;
        t.a = std::cos(r);
        t.b = std::sin(r);
        t.c = -std::sin(r);
        t.d = std::cos(r);
        _transform = t.then(_transform);
    }

    void scale(double s) override {
        Affine t;
        t.a = s;
        t.d = s;
        _transform = t.then(_transform);
    }

    double scale() override {
        return _transform.a;
    }

    void reset_transformations() override {
        _transform = Affine();
        _stack.clear();
    }

    void getTranslate(double* x, double* y) override {
        *x = _transform.e;
        *y = _transform.f;
    }

    void move_to(double x, double y) override {
        _penX = x;
        _penY = y;
        _hasStart = true;
        _transform.apply(x, y, &_startX, &_startY);
    }

    void line_to(double x, double y) override {
        double endX = 0.0;
        double endY = 0.0;
        _transform.apply(x, y, &endX, &endY);
        if (_hasStart) {
            lines.push_back(DrawnLine{_startX, _startY, endX, endY});
        }
        _penX = x;
        _penY = y;
        _startX = endX;
        _startY = endY;
    }

    void font_size(double size, bool /*deviceCoords*/) override {
        _fontSize = size;
    }

    void text(const char* text_val) override {
        double x = 0.0;
        double y = 0.0;
        _transform.apply(_penX, _penY, &x, &y);
        texts.push_back(DrawnText{text_val == nullptr ? "" : text_val, x, y, _fontSize});
    }

    TextExtends text_extends(const char* text_val) override {
        TextExtends extends{};
        const std::size_t length = text_val == nullptr ? 0 : std::strlen(text_val);
        extends.width = static_cast<double>(length) * _fontSize * 0.5;
        extends.height = _fontSize;
        extends.x_advance = extends.width;
        extends.y_advance = 0.0;
        return extends;
    }

    void select_font_face(const char* /*text_val*/, const char* /*font_type*/) override {}

    // -- the rest: a painter that draws nothing -------------------------
    void create_resources() override {}
    void new_device_size(unsigned int /*width*/, unsigned int /*height*/) override {}
    void new_path() override {}
    void close_path() override {}
    void new_sub_path() override {}
    void clear(double /*r*/, double /*g*/, double /*b*/) override {}
    void clear(double /*r*/, double /*g*/, double /*b*/, double /*a*/) override {}
    void lineWidthCompensation(double /*lwc*/) override {}
    void line_width(double /*lineWidth*/) override {}
    void arc(double /*x*/, double /*y*/, double /*r*/, double /*start*/, double /*end*/) override {}
    void arcNegative(double /*x*/, double /*y*/, double /*r*/, double /*start*/, double /*end*/) override {}
    void circle(double /*x*/, double /*y*/, double /*r*/) override {}
    void ellipse(double /*cx*/, double /*cy*/, double /*rx*/, double /*ry*/,
                 double /*sa*/, double /*ea*/, double /*ra*/) override {}
    void rectangle(double /*x1*/, double /*y1*/, double /*w*/, double /*h*/) override {}
    void stroke() override { _hasStart = false; }
    void source_rgb(double /*r*/, double /*g*/, double /*b*/) override {}
    void source_rgba(double /*r*/, double /*g*/, double /*b*/, double /*a*/) override {}
    void user_to_device(double* /*x*/, double* /*y*/) override {}
    void device_to_user(double* /*x*/, double* /*y*/) override {}
    void user_to_device_distance(double* /*dx*/, double* /*dy*/) override {}
    void device_to_user_distance(double* /*dx*/, double* /*dy*/) override {}
    void quadratic_curve_to(double /*x1*/, double /*y1*/, double /*x2*/, double /*y2*/) override {}
    void curve_to(double /*x1*/, double /*y1*/, double /*x2*/, double /*y2*/,
                  double /*x3*/, double /*y3*/) override {}
    long pattern_create_linear(double /*x1*/, double /*y1*/, double /*x2*/, double /*y2*/) override {
        return 0;
    }
    void pattern_add_color_stop_rgba(long /*pat*/, double /*offset*/, double /*r*/,
                                     double /*g*/, double /*b*/, double /*a*/) override {}
    void set_pattern_source(long /*pat*/) override {}
    void pattern_destroy(long /*pat*/) override {}
    void fill() override {}
    void point(double /*x*/, double /*y*/, double /*size*/, bool /*deviceCoords*/) override {}
    unsigned char* data() override { return nullptr; }
    void set_dash(const double* /*dashes*/, const int /*num_dashes*/, double /*offset*/,
                  bool /*scaled*/) override {}
    void dash_destroy() override {}
    long image_create(const std::string& /*file*/) override { return 0; }
    void image_destroy(long /*image*/) override {}
    void image(long /*image*/, double /*uvx*/, double /*vy*/, double /*vvx*/,
               double /*vvy*/, double /*x*/, double /*y*/) override {}
    void disable_antialias() override {}
    void enable_antialias() override {}
    bool isCachingEnabled() override { return false; }
    void startcaching() override {}
    void finishcaching(unsigned long /*id*/) override {}
    LcPainter* getCacherpainter() override { return this; }
    bool isEntityCached(unsigned long /*id*/) override { return false; }
    void renderEntityCached(unsigned long /*id*/) override {}
    void deleteEntityCached(unsigned long /*id*/) override {}
    std::vector<std::string> getFontList() const override { return {}; }
    void addFontsFromPath(const std::vector<std::string>& /*paths*/) override {}

private:
    Affine _transform;
    std::vector<Affine> _stack;
    double _penX{0.0};
    double _penY{0.0};
    double _startX{0.0};
    double _startY{0.0};
    bool _hasStart{false};
    double _fontSize{1.0};
};

}  // namespace testing
}  // namespace viewer
}  // namespace lc
