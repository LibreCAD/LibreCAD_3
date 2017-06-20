#include <gtest/gtest.h>
#include <memory>
#include <cad/meta/layer.h>
#include <cad/primitive/line.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/point.h>
#include <cad/primitive/insert.h>
#include <cad/dochelpers/documentimpl.h>
#include <cad/dochelpers/storagemanagerimpl.h>
#include <cad/meta/dxflinepattern.h>

TEST(BuilderTest, Line) {
    auto layer = std::make_shared<lc::Layer>();
    auto metaInfo = lc::MetaInfo::create();
    auto block = std::make_shared<lc::Block>("Test Block", lc::geo::Coordinate(), nullptr, nullptr, nullptr);

    const lc::geo::Coordinate start(0, 0);
    const lc::geo::Coordinate end(100, 100);

    lc::builder::LineBuilder builder;
    auto line = builder
            .setStart(start)
            ->setEnd(end)
            ->setLayer(layer)
            ->setMetaInfo(metaInfo)
            ->setBlock(block)
            ->build();

    EXPECT_EQ(start, line->start());
    EXPECT_EQ(end, line->end());
    EXPECT_EQ(layer, line->layer());
    EXPECT_EQ(metaInfo, line->metaInfo());
    EXPECT_EQ(block, line->block());
}

TEST(BuilderTest, Arc) {
    auto layer = std::make_shared<lc::Layer>();
    auto metaInfo = lc::MetaInfo::create();
    auto block = std::make_shared<lc::Block>("Test Block", lc::geo::Coordinate(), nullptr, nullptr, nullptr);

    const lc::geo::Coordinate center(0, 0);
    const double radius = 10;
    const double startAngle = 1;
    const double endAngle = 2;
    const bool isCCW = true;

    lc::builder::ArcBuilder builder;
    auto arc = builder
            .setCenter(center)
            ->setRadius(radius)
            ->setStartAngle(startAngle)
            ->setEndAngle(endAngle)
            ->setIsCCW(isCCW)
            ->setLayer(layer)
            ->setMetaInfo(metaInfo)
            ->setBlock(block)
            ->build();

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
    auto layer = std::make_shared<lc::Layer>();
    auto metaInfo = lc::MetaInfo::create();
    auto block = std::make_shared<lc::Block>("Test Block", lc::geo::Coordinate(), nullptr, nullptr, nullptr);

    const lc::geo::Coordinate center(0, 0);
    const double radius = 10;

    lc::builder::CircleBuilder builder;
    auto circle = builder
            .setCenter(center)
            ->setRadius(radius)
            ->setLayer(layer)
            ->setMetaInfo(metaInfo)
            ->setBlock(block)
            ->build();

    EXPECT_EQ(center, circle->center());
    EXPECT_EQ(radius, circle->radius());
    EXPECT_EQ(layer, circle->layer());
    EXPECT_EQ(metaInfo, circle->metaInfo());
    EXPECT_EQ(block, circle->block());
}

TEST(BuilderTest, Point) {
    auto layer = std::make_shared<lc::Layer>();
    auto metaInfo = lc::MetaInfo::create();
    auto block = std::make_shared<lc::Block>("Test Block", lc::geo::Coordinate(), nullptr, nullptr, nullptr);

    const lc::geo::Coordinate coordinate(11, 15);

    lc::builder::PointBuilder builder;
    auto point = builder
            .setCoordinate(coordinate)
            ->setLayer(layer)
            ->setMetaInfo(metaInfo)
            ->setBlock(block)
            ->build();

    EXPECT_EQ(coordinate, *point);
    EXPECT_EQ(layer, point->layer());
    EXPECT_EQ(metaInfo, point->metaInfo());
    EXPECT_EQ(block, point->block());
}

TEST(BuilderTest, Insert) {
    auto layer = std::make_shared<lc::Layer>();
    auto metaInfo = lc::MetaInfo::create();
    auto block = std::make_shared<lc::Block>("Test Block", lc::geo::Coordinate(), nullptr, nullptr, nullptr);
    auto displayBlock = std::make_shared<lc::Block>("Display Block", lc::geo::Coordinate(), nullptr, nullptr, nullptr);
    auto document = std::make_shared<lc::DocumentImpl>(std::make_shared<lc::StorageManagerImpl>());

    const lc::geo::Coordinate coordinate(11, 15);

    lc::builder::InsertBuilder builder;
    auto insert = builder
            .setCoordinate(coordinate)
            ->setLayer(layer)
            ->setMetaInfo(metaInfo)
            ->setBlock(block)
            ->setDisplayBlock(displayBlock)
            ->setDocument(document)
            ->build();

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
    auto lp = std::make_shared<const lc::DxfLinePattern>();
    auto lw = lc::MetaLineWidthByValue(2);
    auto color = lc::Color();

    auto layer = builder
        .setName(name)
        ->setLinePattern(lp)
        ->setLineWidth(lw)
        ->setColor(color)
        ->setIsFrozen(true)
        ->build();

    EXPECT_EQ(name, layer->name());
    EXPECT_EQ(lp, layer->linePattern());
    EXPECT_EQ(lw.width(), layer->lineWidth().width());
    EXPECT_TRUE(color == layer->color());
    EXPECT_TRUE(layer->isFrozen());
}

TEST(BuilderTest, DxfLinePattern) {
    auto builder = lc::builder::LinePatternBuilder();
    auto name = "Test LP";
    auto description = "Test";
    std::vector<double> initialPath = {5, 0, 1, 2};
    double lastElement = 3;

    auto lp = builder
        .setName(name)
        ->setDescription(description)
        ->setPath(initialPath)
        ->addElement(lastElement)
        ->build();

    initialPath.push_back(lastElement);

    EXPECT_EQ(name, lp->name());
    EXPECT_EQ(description, lp->description());
    EXPECT_EQ(initialPath, lp->path());
    EXPECT_EQ(11, lp->length());
}