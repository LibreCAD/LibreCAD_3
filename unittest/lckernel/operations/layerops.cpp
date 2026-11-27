#include <gtest/gtest.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/storage/documentimpl.h>
#include <cad/operations/layerops.h>
#include <cad/operations/entitybuilder.h>
#include <cad/builders/insert.h>
#include <cad/primitive/insert.h>
#include <cad/primitive/image.h>
#include <cad/meta/block.h>

TEST(LayerOps, ReplaceLayer) {
    auto document = std::make_shared<lc::storage::DocumentImpl>(std::make_shared<lc::storage::StorageManagerImpl>());

    auto layer = std::make_shared<lc::meta::Layer>("1");
    auto layer2 = std::make_shared<lc::meta::Layer>("2");
    auto addLayer = std::make_shared<lc::operation::AddLayer>(document, layer);
    addLayer->execute();

    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), layer, nullptr);
    auto addLine = std::make_shared<lc::operation::EntityBuilder>(document);
    addLine->appendEntity(line);
    addLine->execute();
    ASSERT_EQ(layer, document->entityContainer().entityByID(line->id())->layer());

    auto replaceLayer = std::make_shared<lc::operation::ReplaceLayer>(document, layer, layer2);
    replaceLayer->execute();
    ASSERT_EQ(layer2, document->entityContainer().entityByID(line->id())->layer());

    replaceLayer->undo();
    ASSERT_EQ(layer, document->entityContainer().entityByID(line->id())->layer());

    replaceLayer->redo();
    ASSERT_EQ(layer2, document->entityContainer().entityByID(line->id())->layer());
}
TEST(LayerOps, ReplaceAndRemoveCoverBlockEntitiesAndPreserveInsertIdentity) {
    auto document = std::make_shared<lc::storage::DocumentImpl>(std::make_shared<lc::storage::StorageManagerImpl>());
    auto layer = std::make_shared<lc::meta::Layer>("LAYER");
    auto replacement = std::make_shared<lc::meta::Layer>("LAYER", layer->lineWidth(), layer->color(), nullptr, false, true);
    document->addDocumentMetaType(layer);
    auto block = std::make_shared<lc::meta::Block>("CONTENT");
    auto outer = std::make_shared<lc::meta::Block>("OUTER");
    document->addDocumentMetaType(block); document->addDocumentMetaType(outer);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(4, 4), layer, nullptr, block);
    auto entities = std::make_shared<lc::operation::EntityBuilder>(document);
    auto image = std::make_shared<lc::entity::Image>("image.png", lc::geo::Coordinate(0, 0),
        lc::geo::Coordinate(1, 0), lc::geo::Coordinate(0, 1), 2, 3, 50, 50, 0, layer, nullptr, block);
    entities->appendEntity(line); entities->appendEntity(image); entities->execute();
    lc::builder::InsertBuilder nested;
    nested.setDocument(document); nested.setDisplayBlock(block); nested.setLayer(layer); nested.setBlock(outer);
    nested.setCoordinate(lc::geo::Coordinate(2, 3));
    auto insertion = nested.build();
    auto placement = std::make_shared<lc::operation::EntityBuilder>(document);
    placement->appendEntity(insertion); placement->execute();
    const auto lineId = line->id(), insertId = insertion->id(), imageId = image->id();
    auto edit = std::make_shared<lc::operation::ReplaceLayer>(document, layer, replacement);
    edit->execute();
    ASSERT_TRUE(document->entityByID(lineId)); ASSERT_TRUE(document->entityByID(insertId));
    EXPECT_EQ(document->entityByID(lineId)->layer(), replacement);
    EXPECT_EQ(document->entityByID(insertId)->layer(), replacement);
    ASSERT_TRUE(document->entityByID(imageId)); EXPECT_EQ(document->entityByID(imageId)->layer(), replacement);
    edit->undo();
    ASSERT_TRUE(document->entityByID(lineId)); ASSERT_TRUE(document->entityByID(insertId));
    EXPECT_EQ(document->entityByID(lineId)->layer(), layer);
    EXPECT_EQ(document->entityByID(insertId)->layer(), layer);
    ASSERT_TRUE(document->entityByID(imageId)); EXPECT_EQ(document->entityByID(imageId)->layer(), layer);
    edit->redo();
    EXPECT_EQ(document->entityByID(lineId)->layer(), replacement);
    EXPECT_EQ(document->entityByID(insertId)->layer(), replacement);
    auto remove = std::make_shared<lc::operation::RemoveLayer>(document, replacement);
    remove->execute();
    EXPECT_FALSE(document->entityByID(lineId)); EXPECT_FALSE(document->entityByID(insertId));
    EXPECT_FALSE(document->entityByID(imageId));
    remove->undo();
    EXPECT_TRUE(document->entityByID(lineId)); EXPECT_TRUE(document->entityByID(insertId));
    EXPECT_TRUE(document->entityByID(imageId));
}
