#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include <cad/operations/entitybuilder.h>
#include <cad/primitive/line.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>

#include "documentcanvas.h"
#include "painters/lcpainter.h"

namespace {

// Only the transform calls matter to autoScale; every other painter operation
// is a no-op so the test needs neither a GL context nor FreeType/GLEW headers.
class TransformPainter : public lc::viewer::LcPainter {
public:
    double scale() override { return _scale; }

    void scale(double s) override { _scale *= s; }

    void translate(double x, double y) override {
        _tx += x * _scale;
        _ty += y * _scale;
    }

    void reset_transformations() override { _scale = 1.; _tx = _ty = 0.; }

    void save() override { _stack.push_back({_scale, _tx, _ty}); }

    void restore() override {
        if (_stack.empty()) return;
        const State state = _stack.back();
        _stack.pop_back();
        _scale = state.scale;
        _tx = state.tx;
        _ty = state.ty;
    }

    void device_to_user(double* x, double* y) override {
        *x = (*x - _tx) / _scale;
        *y = (*y - _ty) / _scale;
    }

    void create_resources() override {}
    void new_device_size(unsigned int, unsigned int) override {}
    void new_path() override {}
    void close_path() override {}
    void new_sub_path() override {}
    void clear(double, double, double) override {}
    void clear(double, double, double, double) override {}
    void move_to(double, double) override {}
    void line_to(double, double) override {}
    void lineWidthCompensation(double) override {}
    void line_width(double) override {}
    void rotate(double) override {}
    void arc(double, double, double, double, double) override {}
    void arcNegative(double, double, double, double, double) override {}
    void circle(double, double, double) override {}
    void ellipse(double, double, double, double, double, double, double) override {}
    void rectangle(double, double, double, double) override {}
    void stroke() override {}
    void source_rgb(double, double, double) override {}
    void source_rgba(double, double, double, double) override {}
    void user_to_device(double* x, double* y) override {
        *x = *x * _scale + _tx;
        *y = *y * _scale + _ty;
    }
    void user_to_device_distance(double* dx, double* dy) override {
        *dx *= _scale;
        *dy *= _scale;
    }
    void device_to_user_distance(double* dx, double* dy) override {
        *dx /= _scale;
        *dy /= _scale;
    }
    void font_size(double, bool) override {}
    void select_font_face(const char*, const char*) override {}
    void text(const char*) override {}
    lc::viewer::TextExtends text_extends(const char*) override { return {}; }
    void quadratic_curve_to(double, double, double, double) override {}
    void curve_to(double, double, double, double, double, double) override {}
    long pattern_create_linear(double, double, double, double) override { return 0; }
    void pattern_add_color_stop_rgba(long, double, double, double, double, double) override {}
    void set_pattern_source(long) override {}
    void pattern_destroy(long) override {}
    void fill() override {}
    void point(double, double, double, bool) override {}
    unsigned char* data() override { return nullptr; }
    void set_dash(const double*, const int, double, bool) override {}
    void dash_destroy() override {}
    long image_create(const std::string&) override { return 0; }
    void image_destroy(long) override {}
    void image(long, double, double, double, double, double, double) override {}
    void disable_antialias() override {}
    void enable_antialias() override {}
    void getTranslate(double* x, double* y) override {
        *x = _tx;
        *y = _ty;
    }
    bool isCachingEnabled() override { return false; }
    void startcaching() override {}
    void finishcaching(unsigned long) override {}
    lc::viewer::LcPainter* getCacherpainter() override { return nullptr; }
    bool isEntityCached(unsigned long) override { return false; }
    void renderEntityCached(unsigned long) override {}
    void deleteEntityCached(unsigned long) override {}
    std::vector<std::string> getFontList() const override { return {}; }
    void addFontsFromPath(const std::vector<std::string>&) override {}

private:
    struct State {
        double scale;
        double tx;
        double ty;
    };

    double _scale = 1.;
    double _tx = 0.;
    double _ty = 0.;
    std::vector<State> _stack;
};

std::shared_ptr<lc::storage::DocumentImpl> makeDocument() {
    return std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
}

void addLine(const std::shared_ptr<lc::storage::DocumentImpl>& document,
             const lc::geo::Coordinate& start, const lc::geo::Coordinate& end) {
    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(
        start, end, document->layerByName("0")));
    builder->execute();
}

}

// Fitting an empty drawing used to divide 0 by 0 and leave a NaN scale behind,
// after which every coordinate mapped to NaN and the canvas showed only its
// clear colour.
TEST(CanvasScaleTest, EmptyDocumentLeavesScaleFinite) {
    auto document = makeDocument();
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    canvas->newDeviceSize(800, 600);
    TransformPainter painter;

    canvas->autoScale(painter);

    EXPECT_TRUE(std::isfinite(painter.scale()));
    EXPECT_DOUBLE_EQ(1.0, painter.scale());
}

// Before the first resize the widget reports a 0x0 device size. Auto-scaling
// then used to divide by zero even when the drawing did have bounds.
TEST(CanvasScaleTest, ZeroDeviceSizeDoesNotPoisonScale) {
    auto document = makeDocument();
    addLine(document, lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(10, 10, 0));
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    TransformPainter painter;

    canvas->autoScale(painter);

    EXPECT_TRUE(std::isfinite(painter.scale()));
    EXPECT_DOUBLE_EQ(1.0, painter.scale());
}

// A drawing made of a single horizontal line has a zero-height bounding box.
// Fitting it must still produce a usable, finite zoom.
TEST(CanvasScaleTest, DegenerateBoundsStayFinite) {
    auto document = makeDocument();
    addLine(document, lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(100, 0, 0));
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    canvas->newDeviceSize(800, 600);
    TransformPainter painter;

    canvas->autoScale(painter);

    EXPECT_TRUE(std::isfinite(painter.scale()));
    EXPECT_GT(painter.scale(), 0.0);
}
