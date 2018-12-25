#include <gtest/gtest.h>
#include <cad/operations/builder.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/operations/layerops.h>
#include <cad/operations/blockops.h>
#include <cad/operations/entitybuilder.h>

using namespace lc;
using namespace storage;
using namespace meta;

TEST(BuilderTest, Process) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto builder = std::make_shared<lc::operation::Builder>(document, "Test");

    auto layer = std::make_shared<lc::meta::Layer>("1");
    auto addLayer = std::make_shared<lc::operation::AddLayer>(document, layer);

    auto block = std::make_shared<Block>("Block", lc::geo::Coordinate());
    auto addBlock = std::make_shared<lc::operation::AddBlock>(document, block);

    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), layer,  nullptr);
    auto addLine = std::make_shared<lc::operation::EntityBuilder>(document);
    addLine->appendEntity(line);

    builder->append(addLayer);
    builder->append(addBlock);
    builder->append(addLine);

    builder->execute();

    EXPECT_EQ(1, document->blocks().size());
    EXPECT_EQ(block, *document->blocks().begin());

    auto layers = document->allLayers();
    EXPECT_EQ(2, layers.size());
    auto it = layers.begin();
    auto layer1 = it->second;
    it++;
    auto layer2 = it->second;
    EXPECT_TRUE((layer == layer1) ^ (layer == layer2));

    auto entities = document->entityContainer().asVector();
    EXPECT_EQ(1, entities.size());
    EXPECT_EQ(line, *entities.begin());
}

TEST(BuilderTest, Undo) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto builder = std::make_shared<lc::operation::Builder>(document, "Test");

    auto layer = std::make_shared<lc::meta::Layer>("1");
    auto addLayer = std::make_shared<lc::operation::AddLayer>(document, layer);

    auto block = std::make_shared<Block>("Block", lc::geo::Coordinate());
    auto addBlock = std::make_shared<lc::operation::AddBlock>(document, block);

    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), layer, nullptr);
    auto addLine = std::make_shared<lc::operation::EntityBuilder>(document);
    addLine->appendEntity(line);

    builder->append(addLayer);
    builder->append(addBlock);
    builder->append(addLine);

    builder->execute();
    builder->undo();

    EXPECT_EQ(0, document->blocks().size());

    EXPECT_EQ(1, document->allLayers().size());

    auto entities = document->entityContainer().asVector();
    EXPECT_EQ(0, entities.size());
}

TEST(BuilderTest, Redo) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto builder = std::make_shared<lc::operation::Builder>(document, "Test");

    auto layer = std::make_shared<lc::meta::Layer>("1");
    auto addLayer = std::make_shared<lc::operation::AddLayer>(document, layer);

    auto block = std::make_shared<Block>("Block", lc::geo::Coordinate());
    auto addBlock = std::make_shared<lc::operation::AddBlock>(document, block);

    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), layer, nullptr);
    auto addLine = std::make_shared<lc::operation::EntityBuilder>(document);
    addLine->appendEntity(line);

    builder->append(addLayer);
    builder->append(addBlock);
    builder->append(addLine);

    builder->execute();
    builder->undo();
    builder->redo();

    EXPECT_EQ(1, document->blocks().size());
    EXPECT_EQ(block, *document->blocks().begin());

    auto layers = document->allLayers();
    EXPECT_EQ(2, layers.size());
    auto it = layers.begin();
    auto layer1 = it->second;
    it++;
    auto layer2 = it->second;
    EXPECT_TRUE((layer == layer1) ^ (layer == layer2));

    auto entities = document->entityContainer().asVector();
    EXPECT_EQ(1, entities.size());
    EXPECT_EQ(line, *entities.begin());
}
