#include <gtest/gtest.h>
#include <cad/operations/builder.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/operations/layerops.h>
#include <cad/operations/blockops.h>
#include <cad/operations/entitybuilder.h>
#include <cad/operations/entityops.h>
#include <cad/storage/undomanagerimpl.h>

using namespace lc;
using namespace storage;
using namespace meta;

TEST(BuilderTest, Process) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto builder = std::make_shared<lc::operation::Builder>(document, "Test");

    //Remove existing blocks
    for(auto& block : document->blocks()) {
        document->removeDocumentMetaType(block);
    }

    auto layer = std::make_shared<lc::meta::Layer>("1");
    auto addLayer = std::make_shared<lc::operation::AddLayer>(document, layer);

    auto block = std::make_shared<Block>("Block", lc::geo::Coordinate());
    auto addBlock = std::make_shared<lc::operation::AddBlock>(document, block);

    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), layer);
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

    //Remove existing blocks
    for(auto& block : document->blocks()) {
        document->removeDocumentMetaType(block);
    }

    auto layer = std::make_shared<lc::meta::Layer>("1");
    auto addLayer = std::make_shared<lc::operation::AddLayer>(document, layer);

    auto block = std::make_shared<Block>("Block", lc::geo::Coordinate());
    auto addBlock = std::make_shared<lc::operation::AddBlock>(document, block);

    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), layer);
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

    //Remove existing blocks
    for(auto& block : document->blocks()) {
        document->removeDocumentMetaType(block);
    }

    auto layer = std::make_shared<lc::meta::Layer>("1");
    auto addLayer = std::make_shared<lc::operation::AddLayer>(document, layer);

    auto block = std::make_shared<Block>("Block", lc::geo::Coordinate());
    auto addBlock = std::make_shared<lc::operation::AddBlock>(document, block);

    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), layer);
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

TEST(BuilderTest, ReexecutingSameBuilderRemainsOneUndoTransaction) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto undoManager = std::make_shared<UndoManagerImpl>(10);
    document->commitProcessEvent().connect<UndoManagerImpl,
        &UndoManagerImpl::on_CommitProcessEvent>(undoManager.get());

    auto layer = document->layerByName("0");
    ASSERT_NE(nullptr, layer);
    auto originalLine = std::make_shared<lc::entity::Line>(
        lc::geo::Coordinate(0, 0), lc::geo::Coordinate(10, 0), layer);
    document->insertEntity(originalLine);

    auto builder = std::make_shared<lc::operation::Builder>(document, "Drag");
    auto removeOriginal = std::make_shared<lc::operation::EntityBuilder>(document);
    removeOriginal->appendEntity(originalLine);
    removeOriginal->appendOperation(std::make_shared<lc::operation::Push>());
    removeOriginal->appendOperation(std::make_shared<lc::operation::Remove>());
    builder->append(removeOriginal);
    builder->execute();
    EXPECT_EQ(nullptr, document->entityByID(originalLine->id()));

    auto replacementBlock = std::make_shared<Block>("Replacement", lc::geo::Coordinate());
    builder->append(std::make_shared<lc::operation::AddBlock>(document, replacementBlock));
    auto addReplacement = std::make_shared<lc::operation::EntityBuilder>(document);
    auto replacementLine = std::make_shared<lc::entity::Line>(
        lc::geo::Coordinate(0, 0), lc::geo::Coordinate(10, 10), layer);
    addReplacement->appendEntity(replacementLine);
    builder->append(addReplacement);
    builder->execute();

    EXPECT_EQ(nullptr, document->entityByID(originalLine->id()));
    EXPECT_NE(nullptr, document->entityByID(replacementLine->id()));
    EXPECT_NE(nullptr, document->blockByName("Replacement"));
    ASSERT_TRUE(undoManager->canUndo());
    undoManager->undo();
    EXPECT_NE(nullptr, document->entityByID(originalLine->id()));
    EXPECT_EQ(nullptr, document->entityByID(replacementLine->id()));
    EXPECT_EQ(nullptr, document->blockByName("Replacement"));
    EXPECT_FALSE(undoManager->canUndo());

    ASSERT_TRUE(undoManager->canRedo());
    undoManager->redo();
    EXPECT_EQ(nullptr, document->entityByID(originalLine->id()));
    EXPECT_NE(nullptr, document->entityByID(replacementLine->id()));
    EXPECT_NE(nullptr, document->blockByName("Replacement"));
}
