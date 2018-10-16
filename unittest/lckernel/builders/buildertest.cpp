#include <gtest/gtest.h>
#include <memory>
#include <cad/meta/layer.h>
#include <cad/primitive/line.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/point.h>
#include <cad/primitive/insert.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/meta/dxflinepattern.h>

TEST(BuilderTest, Line) {
    auto layer = std::make_shared<lc::meta::Layer>();
    auto metaInfo = lc::meta::MetaInfo::create();
    auto block = std::make_shared<lc::meta::Block>("Test Block", lc::geo::Coordinate());

    const lc::geo::Coordinate start(0, 0);
    const lc::geo::Coordinate end(100, 100);

    lc::builder::LineBuilder builder;
    builder.setStart(start);
    builder.setEnd(end);
    builder.setLayer(layer);
    builder.setMetaInfo(metaInfo);
    builder.setBlock(block);
    auto line = builder.build();

    EXPECT_EQ(start, line->start());
    EXPECT_EQ(end, line->end());
    EXPECT_EQ(layer, line->layer());
    EXPECT_EQ(metaInfo, line->metaInfo());
    EXPECT_EQ(block, line->block());
}

TEST(BuilderTest, Arc) {
    auto layer = std::make_shared<lc::meta::Layer>();
    auto metaInfo = lc::meta::MetaInfo::create();
    auto block = std::make_shared<lc::meta::Block>("Test Block", lc::geo::Coordinate());

    const lc::geo::Coordinate center(0, 0);
    const double radius = 10;
    const double startAngle = 1;
    const double endAngle = 2;
    const bool isCCW = true;

    lc::builder::ArcBuilder builder;
    builder.setCenter(center);
    builder.setRadius(radius);
    builder.setStartAngle(startAngle);
    builder.setEndAngle(endAngle);
    builder.setIsCCW(isCCW);
    builder.setLayer(layer);
    builder.setMetaInfo(metaInfo);
    builder.setBlock(block);
    auto arc = builder.build();

    EXPECT_EQ(center, arc->center());
    EXPECT_EQ(radius, arc->radius());
    EXPECT_EQ(startAngle, arc->startAngle());
    EXPECT_EQ(endAngle, arc->endAngle());
    EXPECT_EQ(isCCW, arc->CCW());
    EXPECT_EQ(layer, arc->layer());
    EXPECT_EQ(metaInfo, arc->metaInfo());
    EXPECT_EQ(block, arc->block());
}

TEST(BuilderTest, Circle) {
    auto layer = std::make_shared<lc::meta::Layer>();
    auto metaInfo = lc::meta::MetaInfo::create();
    auto block = std::make_shared<lc::meta::Block>("Test Block", lc::geo::Coordinate());

    const lc::geo::Coordinate center(0, 0);
    const double radius = 10;

    lc::builder::CircleBuilder builder;
    builder.setCenter(center);
    builder.setRadius(radius);
    builder.setLayer(layer);
    builder.setMetaInfo(metaInfo);
    builder.setBlock(block);
    auto circle = builder.build();

    EXPECT_EQ(center, circle->center());
    EXPECT_EQ(radius, circle->radius());
    EXPECT_EQ(layer, circle->layer());
    EXPECT_EQ(metaInfo, circle->metaInfo());
    EXPECT_EQ(block, circle->block());
}

TEST(BuilderTest, Point) {
    auto layer = std::make_shared<lc::meta::Layer>();
    auto metaInfo = lc::meta::MetaInfo::create();
    auto block = std::make_shared<lc::meta::Block>("Test Block", lc::geo::Coordinate());

    const lc::geo::Coordinate coordinate(11, 15);

    lc::builder::PointBuilder builder;
    builder.setCoordinate(coordinate);
    builder.setLayer(layer);
    builder.setMetaInfo(metaInfo);
    builder.setBlock(block);
    auto point = builder.build();

    EXPECT_EQ(coordinate, *point);
    EXPECT_EQ(layer, point->layer());
    EXPECT_EQ(metaInfo, point->metaInfo());
    EXPECT_EQ(block, point->block());
}

TEST(BuilderTest, Insert) {
    auto layer = std::make_shared<lc::meta::Layer>();
    auto metaInfo = lc::meta::MetaInfo::create();
    auto block = std::make_shared<lc::meta::Block>("Test Block", lc::geo::Coordinate());
    auto displayBlock = std::make_shared<lc::meta::Block>("Display Block", lc::geo::Coordinate());
    auto document = std::make_shared<lc::storage::DocumentImpl>(std::make_shared<lc::storage::StorageManagerImpl>());

    const lc::geo::Coordinate coordinate(11, 15);

    lc::builder::InsertBuilder builder;
    builder.setCoordinate(coordinate);
    builder.setLayer(layer);
    builder.setMetaInfo(metaInfo);
    builder.setBlock(block);
    builder.setDisplayBlock(displayBlock);
    builder.setDocument(document);
    auto insert = builder.build();

    EXPECT_EQ(coordinate, insert->position());
    EXPECT_EQ(layer, insert->layer());
    EXPECT_EQ(metaInfo, insert->metaInfo());
    EXPECT_EQ(block, insert->block());
    EXPECT_EQ(document, insert->document());
    EXPECT_EQ(displayBlock, insert->displayBlock());
}

TEST(BuilderTest, Layer) {
    auto builder = lc::builder::LayerBuilder();
    auto name = "Test Layer";
    auto lp = std::make_shared<const lc::meta::DxfLinePatternByValue>();
    auto lw = lc::meta::MetaLineWidthByValue(2);
    auto color = lc::Color();

    builder.setName(name);
    builder.setLinePattern(lp);
    builder.setLineWidth(lw);
    builder.setColor(color);
    builder.setIsFrozen(true);
    auto layer = builder.build();

    EXPECT_EQ(name, layer->name());
    EXPECT_EQ(lp, layer->linePattern());
    EXPECT_EQ(lw.width(), layer->lineWidth().width());
    EXPECT_TRUE(color == layer->color());
    EXPECT_TRUE(layer->isFrozen());
}

TEST(BuilderTest, DxfLinePatternByValue) {
    auto builder = lc::builder::LinePatternBuilder();
    auto name = "Test LP";
    auto description = "Test";
    std::vector<double> initialPath = {5, 0, 1, 2};
    double lastElement = 3;

    builder.setName(name);
    builder.setDescription(description);
    builder.setPath(initialPath);
    builder.addElement(lastElement);
    auto lp = builder.build();

    initialPath.push_back(lastElement);

    EXPECT_EQ(name, lp->name());
    EXPECT_EQ(description, lp->description());
    EXPECT_EQ(initialPath, lp->path());
    EXPECT_EQ(11, lp->length());
}