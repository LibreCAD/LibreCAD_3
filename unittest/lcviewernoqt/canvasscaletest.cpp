#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include <cad/operations/entitybuilder.h>
#include <cad/operations/builder.h>
#include <cad/operations/layerops.h>
#include <cad/primitive/line.h>
#include <cad/primitive/point.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/image.h>
#include <cad/primitive/hatch.h>
#include <cad/primitive/insert.h>
#include <cad/builders/insert.h>
#include <cad/meta/block.h>
#include <cad/meta/metacolor.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>

#include "documentcanvas.h"
#include "painters/lcpainter.h"
#include "managers/snapmanagerimpl.h"
#include "events/snappointevent.h"

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

TEST(CanvasScaleTest, SolidFillColorIsIndependentAndSelectionTakesPrecedence) {
    class ColorPainter : public TransformPainter {
    public:
        void source_rgba(double red, double green, double blue, double alpha) override {
            color = lc::Color(red, green, blue, alpha);
        }
        lc::Color color;
    };
    class EmptyDrawable : public lc::viewer::LCVDrawItem {
    public:
        explicit EmptyDrawable(const lc::entity::CADEntity_CSPtr& entity)
            : LCVDrawItem(entity, true), _entity(entity) {}
        lc::entity::CADEntity_CSPtr entity() const override { return _entity; }
    private:
        lc::entity::CADEntity_CSPtr _entity;
    };
    auto doc = makeDocument();
    auto meta = lc::meta::MetaInfo::create();
    meta->add(std::make_shared<lc::meta::MetaColorByValue>(255, 0, 0));
    meta->add(std::make_shared<lc::meta::MetaFillColor>(lc::Color(0, 255, 0)));
    auto hatch = std::make_shared<lc::entity::Hatch>(doc->layerByName("0"), meta);
    hatch->setSolid(1);
    auto item = std::make_shared<EmptyDrawable>(hatch);
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc);
    ColorPainter painter;
    canvas->drawEntity(painter, item);
    EXPECT_EQ(painter.color.redI(), 0); EXPECT_EQ(painter.color.greenI(), 255); EXPECT_EQ(painter.color.blueI(), 0);
    canvas->drawCachedEntity(painter, item);
    EXPECT_EQ(painter.color.greenI(), 255);
    item->selected(true);
    canvas->drawEntity(painter, item);
    EXPECT_EQ(painter.color.redI(), 0); EXPECT_EQ(painter.color.greenI(), 0); EXPECT_EQ(painter.color.blueI(), 255);
    item->selected(false); hatch->setSolid(0);
    canvas->drawEntity(painter, item);
    EXPECT_EQ(painter.color.redI(), 255); EXPECT_EQ(painter.color.greenI(), 0); EXPECT_EQ(painter.color.blueI(), 0);
}

TEST(CanvasScaleTest, OffLayersAreNotDrawnSelectedOrUsedForFit) {
    class CountingPainter : public TransformPainter {
    public:
        void source_rgba(double, double, double, double) override { ++draws; }
        void renderEntityCached(unsigned long) override { ++cached; }
        int draws{0}, cached{0};
    };
    auto doc = makeDocument();
    auto hidden = std::make_shared<lc::meta::Layer>("OFF", lc::meta::MetaLineWidthByValue(1),
        lc::Color(255, 0, 0), nullptr, false, true);
    doc->addDocumentMetaType(hidden);
    auto visibleLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0), lc::geo::Coordinate(100,0), doc->layerByName("0"));
    auto hiddenLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,10000), lc::geo::Coordinate(100,10000), hidden);
    auto builder = std::make_shared<lc::operation::EntityBuilder>(doc);
    builder->appendEntity(visibleLine); builder->appendEntity(hiddenLine); builder->execute();
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc); canvas->newDeviceSize(800,600);
    CountingPainter painter;
    auto hiddenItem = canvas->getDrawable(hiddenLine); ASSERT_TRUE(hiddenItem);
    canvas->drawEntity(painter, hiddenItem); canvas->drawCachedEntity(painter, hiddenItem);
    EXPECT_EQ(painter.draws, 0); EXPECT_EQ(painter.cached, 0);
    canvas->drawEntity(painter, canvas->getDrawable(visibleLine)); EXPECT_EQ(painter.draws, 1);
    canvas->autoScale(painter); EXPECT_GT(painter.scale(), 1.0);
    canvas->selectAll(); ASSERT_EQ(canvas->selectedDrawables().size(), 1u);
    EXPECT_EQ(canvas->selectedDrawables()[0]->entity()->id(), visibleLine->id());
    canvas->removeSelection(); canvas->inverseSelection(); ASSERT_EQ(canvas->selectedDrawables().size(), 1u);
    canvas->removeSelection(); canvas->selectEntity(hiddenLine); EXPECT_TRUE(canvas->selectedDrawables().empty());
    canvas->selectPoint(50, 10000); EXPECT_TRUE(canvas->selectedDrawables().empty());
    canvas->makeSelection(-10, 9990, 120, 20, false); canvas->closeSelection();
    EXPECT_TRUE(canvas->selectedDrawables().empty());
    // Hiding a layer drops an already selected entity after the replacement commits.
    canvas->selectEntity(visibleLine); ASSERT_EQ(canvas->selectedDrawables().size(), 1u);
    auto old = doc->layerByName("0");
    auto off = std::make_shared<lc::meta::Layer>("0", old->lineWidth(), old->color(), old->linePattern(), old->isFrozen(), true);
    auto edit = std::make_shared<lc::operation::Builder>(doc, "turn layer off");
    edit->append(std::make_shared<lc::operation::ReplaceLayer>(doc, old, off)); edit->execute();
    EXPECT_TRUE(canvas->selectedDrawables().empty());
}

TEST(CanvasScaleTest, OffLayerEntitiesDoNotSnap) {
    struct Receiver {
        lc::geo::Coordinate point;
        void onSnap(const lc::viewer::event::SnapPointEvent& event) { point = event.snapPoint(); }
    } receiver;
    auto doc = makeDocument();
    auto off = std::make_shared<lc::meta::Layer>("OFF", lc::meta::MetaLineWidthByValue(1),
        lc::Color(255,0,0), nullptr, false, true);
    doc->addDocumentMetaType(off);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0), lc::geo::Coordinate(10,10), off);
    auto builder = std::make_shared<lc::operation::EntityBuilder>(doc); builder->appendEntity(line); builder->execute();
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc);
    lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10);
    snap.setEntitySnappable(true);
    snap.snapPointEvents().connect<Receiver, &Receiver::onSnap>(&receiver);
    snap.setDeviceLocation(2, 3);
    EXPECT_DOUBLE_EQ(receiver.point.x(), 2); EXPECT_DOUBLE_EQ(receiver.point.y(), 3);
    auto on = std::make_shared<lc::meta::Layer>("OFF", off->lineWidth(), off->color(), off->linePattern(), false, false);
    auto edit = std::make_shared<lc::operation::Builder>(doc, "turn layer on");
    edit->append(std::make_shared<lc::operation::ReplaceLayer>(doc, off, on)); edit->execute();
    snap.setDeviceLocation(2, 3);
    EXPECT_NEAR(receiver.point.x(), 2.5, 1e-12); EXPECT_NEAR(receiver.point.y(), 2.5, 1e-12);
    snap.snapPointEvents().disconnect<Receiver, &Receiver::onSnap>(&receiver);
}

TEST(CanvasScaleTest, NestedInsertLayerZeroInheritsVisibility) {
    class CountingPainter : public TransformPainter {
    public:
        void source_rgba(double, double, double, double) override { ++draws; }
        int draws{0};
    } painter;
    auto doc = makeDocument(); auto originalZero = doc->layerByName("0");
    auto zeroFrozen = std::make_shared<lc::meta::Layer>("0", originalZero->lineWidth(), originalZero->color(), nullptr, true, false);
    auto edit = std::make_shared<lc::operation::Builder>(doc, "freeze layer zero");
    edit->append(std::make_shared<lc::operation::ReplaceLayer>(doc, originalZero, zeroFrozen)); edit->execute();
    auto on = std::make_shared<lc::meta::Layer>("ON", lc::meta::MetaLineWidthByValue(1), lc::Color(255,255,255), nullptr, false, false);
    auto off = std::make_shared<lc::meta::Layer>("OFF", lc::meta::MetaLineWidthByValue(1), lc::Color(255,0,0), nullptr, false, true);
    doc->addDocumentMetaType(on); doc->addDocumentMetaType(off);
    auto outer = std::make_shared<lc::meta::Block>("OUTER"); auto inner = std::make_shared<lc::meta::Block>("INNER");
    doc->addDocumentMetaType(outer); doc->addDocumentMetaType(inner);
    auto children = std::make_shared<lc::operation::EntityBuilder>(doc);
    children->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0), lc::geo::Coordinate(3,3), zeroFrozen, nullptr, inner));
    children->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,10000), lc::geo::Coordinate(3,10003), off, nullptr, outer));
    children->execute();
    lc::builder::InsertBuilder nested; nested.setDocument(doc); nested.setDisplayBlock(inner); nested.setLayer(zeroFrozen); nested.setBlock(outer);
    nested.setCoordinate(lc::geo::Coordinate(0,0));
    auto nestedInsert = nested.build();
    auto place = std::make_shared<lc::operation::EntityBuilder>(doc); place->appendEntity(nestedInsert); place->execute();
    lc::builder::InsertBuilder top; top.setDocument(doc); top.setDisplayBlock(outer); top.setLayer(on); top.setCoordinate(lc::geo::Coordinate(0,0));
    auto parent = top.build();
    auto parentPlacement = std::make_shared<lc::operation::EntityBuilder>(doc);
    parentPlacement->appendEntity(parent); parentPlacement->execute();
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc); canvas->newDeviceSize(800,600);
    auto drawable = lc::viewer::DocumentCanvas::asDrawable(parent); ASSERT_TRUE(drawable);
    canvas->drawEntity(painter, drawable);
    EXPECT_EQ(painter.draws, 1);
    canvas->autoScale(painter);
    EXPECT_GT(painter.scale(), 1.0);
}

TEST(CanvasScaleTest, FitExcludesHiddenInsertChildren) {
    class CountingPainter : public TransformPainter {
    public:
        void source_rgba(double, double, double, double) override { ++draws; }
        int draws{0};
    };
    auto doc = makeDocument();
    auto off = std::make_shared<lc::meta::Layer>("OFF", lc::meta::MetaLineWidthByValue(1),
        lc::Color(255, 0, 0), nullptr, false, true);
    doc->addDocumentMetaType(off);
    auto block = std::make_shared<lc::meta::Block>("FIT_BLOCK");
    doc->addDocumentMetaType(block);
    auto entities = std::make_shared<lc::operation::EntityBuilder>(doc);
    entities->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
        lc::geo::Coordinate(100, 0), doc->layerByName("0"), nullptr, block));
    entities->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 10000),
        lc::geo::Coordinate(100, 10000), off, nullptr, block));
    entities->execute();
    lc::builder::InsertBuilder insert;
    insert.setDocument(doc); insert.setDisplayBlock(block); insert.setLayer(doc->layerByName("0"));
    insert.setCoordinate(lc::geo::Coordinate(10, 20));
    auto placement = std::make_shared<lc::operation::EntityBuilder>(doc);
    auto parent = insert.build(); placement->appendEntity(parent); placement->execute();
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc); canvas->newDeviceSize(800, 600);
    auto drawable = canvas->getDrawable(parent); ASSERT_TRUE(drawable);
    CountingPainter initiallyDrawn;
    canvas->drawEntity(initiallyDrawn, drawable); EXPECT_EQ(initiallyDrawn.draws, 1);
    TransformPainter visibleOnly;
    canvas->autoScale(visibleOnly);
    EXPECT_GT(visibleOnly.scale(), 1.0);

    auto on = std::make_shared<lc::meta::Layer>("OFF", off->lineWidth(), off->color(), nullptr, false, false);
    auto edit = std::make_shared<lc::operation::Builder>(doc, "turn layer on");
    edit->append(std::make_shared<lc::operation::ReplaceLayer>(doc, off, on)); edit->execute();
    CountingPainter afterToggle;
    canvas->drawEntity(afterToggle, drawable); EXPECT_EQ(afterToggle.draws, 2);
    TransformPainter allVisible;
    canvas->autoScale(allVisible);
    EXPECT_LT(allVisible.scale(), 1.0);
    edit->undo();
    TransformPainter hiddenAgain;
    canvas->autoScale(hiddenAgain);
    EXPECT_GT(hiddenAgain.scale(), 1.0);
    edit->redo();
    TransformPainter visibleAgain;
    canvas->autoScale(visibleAgain);
    EXPECT_LT(visibleAgain.scale(), 1.0);
}

TEST(CanvasScaleTest, FrozenAndLockedLayersHaveDifferentBehavior) {
    class CountingPainter : public TransformPainter {
    public:
        void source_rgba(double, double, double, double) override { ++draws; }
        int draws{0};
    } painter;
    struct Receiver {
        lc::geo::Coordinate point;
        void onSnap(const lc::viewer::event::SnapPointEvent& event) { point = event.snapPoint(); }
    } receiver;
    auto doc = makeDocument();
    auto frozen = std::make_shared<lc::meta::Layer>("FROZEN", lc::meta::MetaLineWidthByValue(1),
        lc::Color(255, 0, 0), nullptr, true, false, false);
    auto locked = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1),
        lc::Color(0, 255, 0), nullptr, false, false, true);
    doc->addDocumentMetaType(frozen); doc->addDocumentMetaType(locked);
    auto visibleLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(10, 0), doc->layerByName("0"));
    auto lockedLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 10), lc::geo::Coordinate(10, 20), locked);
    auto frozenLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 10000), lc::geo::Coordinate(10, 10010), frozen);
    auto builder = std::make_shared<lc::operation::EntityBuilder>(doc);
    builder->appendEntity(visibleLine); builder->appendEntity(lockedLine); builder->appendEntity(frozenLine); builder->execute();
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc); canvas->newDeviceSize(800, 600);
    canvas->drawEntity(painter, canvas->getDrawable(lockedLine)); EXPECT_EQ(painter.draws, 1);
    canvas->drawEntity(painter, canvas->getDrawable(frozenLine)); EXPECT_EQ(painter.draws, 1);
    canvas->autoScale(painter); EXPECT_GT(painter.scale(), 1.0);
    canvas->selectAll(); ASSERT_EQ(canvas->selectedDrawables().size(), 1u);
    EXPECT_EQ(canvas->selectedDrawables()[0]->entity()->id(), visibleLine->id());
    canvas->removeSelection(); canvas->selectEntity(lockedLine); EXPECT_TRUE(canvas->selectedDrawables().empty());
    canvas->selectEntity(frozenLine); EXPECT_TRUE(canvas->selectedDrawables().empty());
    lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10);
    snap.setEntitySnappable(true); snap.snapPointEvents().connect<Receiver, &Receiver::onSnap>(&receiver);
    snap.setDeviceLocation(2, 13);
    EXPECT_NEAR(receiver.point.x(), 2.5, 1e-12); EXPECT_NEAR(receiver.point.y(), 12.5, 1e-12);
    snap.setDeviceLocation(2, 10003);
    EXPECT_DOUBLE_EQ(receiver.point.x(), 2); EXPECT_DOUBLE_EQ(receiver.point.y(), 10003);
    snap.snapPointEvents().disconnect<Receiver, &Receiver::onSnap>(&receiver);
}

TEST(CanvasScaleTest, ElevatedPointCanBeSelectedAndSnappedInPlanarView) {
    struct Receiver {
        lc::geo::Coordinate point;
        void onSnap(const lc::viewer::event::SnapPointEvent& event) { point = event.snapPoint(); }
    } receiver;
    auto doc = makeDocument();
    auto point = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(2, 3, 70), doc->layerByName("0"));
    auto insert = std::make_shared<lc::operation::EntityBuilder>(doc); insert->appendEntity(point); insert->execute();
    const auto flags = lc::SimpleSnapConstrain();
    using Container = lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>;
    using OriginalQuery = std::vector<lc::EntityDistance> (Container::*)(
        const lc::geo::Coordinate&, double, lc::SimpleSnapConstrain) const;
    const auto originalQuery = static_cast<OriginalQuery>(&Container::getEntityPathsNearCoordinate);
    EXPECT_TRUE((doc->entityContainer().*originalQuery)(lc::geo::Coordinate(2, 3), 1, flags).empty());
    EXPECT_EQ(doc->entityContainer().getEntityPathsNearCoordinate(lc::geo::Coordinate(2, 3, 70), 1, flags).size(), 1u);
    EXPECT_EQ(doc->entityContainer().getEntityPathsNearCoordinatePlanar(lc::geo::Coordinate(2, 3), 1, flags).size(), 1u);
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc);
    canvas->selectPoint(2, 3);
    EXPECT_EQ(canvas->selectedDrawables().size(), 1u);
    auto competitor = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(4, 3, 0), doc->layerByName("0"));
    auto addCompetitor = std::make_shared<lc::operation::EntityBuilder>(doc);
    addCompetitor->appendEntity(competitor); addCompetitor->execute();
    lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10);
    snap.setMiddleSnappable(true);
    snap.snapPointEvents().connect<Receiver, &Receiver::onSnap>(&receiver);
    snap.setDeviceLocation(2, 3);
    EXPECT_DOUBLE_EQ(receiver.point.x(), 2); EXPECT_DOUBLE_EQ(receiver.point.y(), 3); EXPECT_DOUBLE_EQ(receiver.point.z(), 70);
    snap.setDeviceLocation(100, 100);
    EXPECT_DOUBLE_EQ(receiver.point.x(), 100); EXPECT_DOUBLE_EQ(receiver.point.y(), 100); EXPECT_DOUBLE_EQ(receiver.point.z(), 0);
    snap.snapPointEvents().disconnect<Receiver, &Receiver::onSnap>(&receiver);
}

TEST(CanvasScaleTest, ElevatedImageCanBeSelectedAndSnappedInPlanarView) {
    struct Receiver {
        lc::geo::Coordinate point;
        void onSnap(const lc::viewer::event::SnapPointEvent& event) { point = event.snapPoint(); }
    } receiver;
    auto doc = makeDocument();
    auto image = std::make_shared<lc::entity::Image>("scratch.png", lc::geo::Coordinate(2, 3, 70),
        lc::geo::Coordinate(1, 0, 0), lc::geo::Coordinate(0, 1, 0), 4, 5, 50, 50, 0,
        doc->layerByName("0"), nullptr);
    auto insert = std::make_shared<lc::operation::EntityBuilder>(doc);
    insert->appendEntity(image); insert->execute();
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc);
    canvas->selectPoint(2, 3);
    ASSERT_EQ(canvas->selectedDrawables().size(), 1u);
    lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10);
    snap.setMiddleSnappable(true);
    snap.snapPointEvents().connect<Receiver, &Receiver::onSnap>(&receiver);
    snap.setDeviceLocation(2, 3);
    EXPECT_EQ(receiver.point, lc::geo::Coordinate(2, 3, 70));
    snap.setDeviceLocation(100, 100);
    EXPECT_EQ(receiver.point, lc::geo::Coordinate(100, 100, 0));
    snap.snapPointEvents().disconnect<Receiver, &Receiver::onSnap>(&receiver);
}

TEST(CanvasScaleTest, TiltedImageCanBeSelectedAndSnappedInPlanarView) {
    struct Receiver {
        lc::geo::Coordinate point;
        void onSnap(const lc::viewer::event::SnapPointEvent& event) { point = event.snapPoint(); }
    } receiver;
    auto doc = makeDocument();
    auto image = std::make_shared<lc::entity::Image>("scratch.png", lc::geo::Coordinate(2, 3, 70),
        lc::geo::Coordinate(1, 0, 1), lc::geo::Coordinate(0, 1, 0), 4, 5, 50, 50, 0,
        doc->layerByName("0"), nullptr);
    auto insert = std::make_shared<lc::operation::EntityBuilder>(doc);
    insert->appendEntity(image); insert->execute();
    const auto found = doc->entityContainer().getEntityPathsNearCoordinatePlanar(
        lc::geo::Coordinate(4, 3), 0.1, lc::SimpleSnapConstrain());
    ASSERT_EQ(found.size(), 1u);
    EXPECT_EQ(found.front().coordinate(), lc::geo::Coordinate(4, 3, 72));
    EXPECT_TRUE(doc->entityContainer().getEntityPathsNearCoordinatePlanar(
        lc::geo::Coordinate(4, 4), 0.1, lc::SimpleSnapConstrain()).empty());
    EXPECT_EQ(image->nearestPointOnPath(lc::geo::Coordinate(4, 3, 0)), lc::geo::Coordinate(2, 3, 70));
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc);
    canvas->selectPoint(4, 3);
    ASSERT_EQ(canvas->selectedDrawables().size(), 1u);
    lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10);
    snap.setEntitySnappable(true);
    snap.snapPointEvents().connect<Receiver, &Receiver::onSnap>(&receiver);
    snap.setDeviceLocation(4, 3);
    EXPECT_EQ(receiver.point, lc::geo::Coordinate(4, 3, 72));
    snap.snapPointEvents().disconnect<Receiver, &Receiver::onSnap>(&receiver);
}

TEST(CanvasScaleTest, ElevatedCircleAndArcCanBeSelectedAndSnappedInPlanarView) {
    struct Receiver {
        lc::geo::Coordinate point;
        void onSnap(const lc::viewer::event::SnapPointEvent& event) { point = event.snapPoint(); }
    };
    for (bool arc : {false, true}) {
        auto doc = makeDocument();
        lc::entity::CADEntity_CSPtr entity;
        if (arc) {
            entity = std::make_shared<lc::entity::Arc>(lc::geo::Coordinate(2, 3, 70), 5,
                    -M_PI / 4., M_PI / 4., true, doc->layerByName("0"));
        } else {
            entity = std::make_shared<lc::entity::Circle>(lc::geo::Coordinate(2, 3, 70), 5,
                    doc->layerByName("0"));
        }
        auto insert = std::make_shared<lc::operation::EntityBuilder>(doc);
        insert->appendEntity(entity); insert->execute();
        auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc);
        canvas->selectPoint(7, 3);
        ASSERT_EQ(canvas->selectedDrawables().size(), 1u);
        Receiver receiver;
        lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10);
        snap.setEntitySnappable(true);
        snap.snapPointEvents().connect<Receiver, &Receiver::onSnap>(&receiver);
        snap.setDeviceLocation(7, 3);
        EXPECT_NEAR(receiver.point.x(), 7, 1e-12);
        EXPECT_NEAR(receiver.point.y(), 3, 1e-12);
        EXPECT_DOUBLE_EQ(receiver.point.z(), 70);
        snap.snapPointEvents().disconnect<Receiver, &Receiver::onSnap>(&receiver);
    }
}

TEST(CanvasScaleTest, ElevatedEllipseCanBeSelectedAndSnappedInPlanarView) {
    struct Receiver {
        lc::geo::Coordinate point;
        void onSnap(const lc::viewer::event::SnapPointEvent& event) { point = event.snapPoint(); }
    } receiver;
    auto doc = makeDocument();
    auto ellipse = std::make_shared<lc::entity::Ellipse>(lc::geo::Coordinate(2, 3, 70),
            lc::geo::Coordinate(5, 0), 2., 0., 2. * M_PI, false, doc->layerByName("0"));
    auto insert = std::make_shared<lc::operation::EntityBuilder>(doc);
    insert->appendEntity(ellipse); insert->execute();
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(doc);
    canvas->selectPoint(7, 3);
    ASSERT_EQ(canvas->selectedDrawables().size(), 1u);
    lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10);
    snap.setEntitySnappable(true);
    snap.snapPointEvents().connect<Receiver, &Receiver::onSnap>(&receiver);
    snap.setDeviceLocation(7, 3);
    EXPECT_NEAR(receiver.point.x(), 7, 1e-12);
    EXPECT_NEAR(receiver.point.y(), 3, 1e-12);
    EXPECT_DOUBLE_EQ(receiver.point.z(), 70);
    snap.snapPointEvents().disconnect<Receiver, &Receiver::onSnap>(&receiver);
}
