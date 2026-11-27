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

TEST(BuilderTest, LockedChildRefusalLeavesCompoundAndUndoHistoryUnchanged) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto history = std::make_shared<UndoManagerImpl>(10);
    document->commitProcessEvent().connect<UndoManagerImpl, &UndoManagerImpl::on_CommitProcessEvent>(history.get());
    auto lockedLayer = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1), lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(lockedLayer);
    auto locked = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), lockedLayer);
    document->insertEntity(locked);
    auto freeLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 1), lc::geo::Coordinate(1, 1), document->layerByName("0"));
    auto add = std::make_shared<lc::operation::EntityBuilder>(document);
    add->appendEntity(freeLine); add->execute();
    history->undo(); ASSERT_TRUE(history->canRedo());

    auto group = std::make_shared<lc::operation::Builder>(document, "Mixed");
    auto newLayer = std::make_shared<lc::meta::Layer>("PARTIAL");
    group->append(std::make_shared<lc::operation::AddLayer>(document, newLayer));
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(locked->move(lc::geo::Coordinate(4, 0)));
    group->append(edit);
    group->execute();

    EXPECT_TRUE(edit->blockedByLockedLayer());
    EXPECT_TRUE(group->blockedByLockedLayer());
    group->undo(); group->redo();
    EXPECT_EQ(document->entityByID(locked->id()), locked);
    EXPECT_EQ(document->layerByName("PARTIAL"), nullptr);
    EXPECT_TRUE(history->canRedo());
    EXPECT_FALSE(history->canUndo());
    if (history->canRedo()) history->redo();
    EXPECT_EQ(document->entityByID(freeLine->id()), freeLine);

    // The same group retains the pending edit and can be retried explicitly.
    edit->setAllowLockedLayerEdits(true);
    group->execute();
    EXPECT_FALSE(group->blockedByLockedLayer());
    EXPECT_EQ(document->layerByName("PARTIAL"), newLayer);
    const auto moved = document->entityByID(locked->id());
    ASSERT_TRUE(moved); EXPECT_NE(moved, locked);
    group->undo();
    EXPECT_EQ(document->layerByName("PARTIAL"), nullptr);
    EXPECT_EQ(document->entityByID(locked->id()), locked);
    group->redo();
    EXPECT_EQ(document->layerByName("PARTIAL"), newLayer);
    EXPECT_EQ(document->entityByID(locked->id()), moved);
}

TEST(BuilderTest, RefusedReuseKeepsPriorCompoundUndoJournal) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto history = std::make_shared<UndoManagerImpl>(10);
    document->commitProcessEvent().connect<UndoManagerImpl, &UndoManagerImpl::on_CommitProcessEvent>(history.get());
    auto lockedLayer = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1), lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(lockedLayer);
    auto locked = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), lockedLayer);
    document->insertEntity(locked);
    auto added = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 1), lc::geo::Coordinate(1, 1), document->layerByName("0"));
    auto group = std::make_shared<lc::operation::Builder>(document, "Reusable");
    auto add = std::make_shared<lc::operation::EntityBuilder>(document); add->appendEntity(added); group->append(add); group->execute();
    ASSERT_EQ(document->entityByID(added->id()), added);
    auto layer = std::make_shared<lc::meta::Layer>("PARTIAL");
    group->append(std::make_shared<lc::operation::AddLayer>(document, layer));
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(locked->move(lc::geo::Coordinate(4, 0))); group->append(edit); group->execute();
    EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_TRUE(edit->blockedByLockedLayer());
    EXPECT_EQ(document->layerByName("PARTIAL"), nullptr);
    ASSERT_TRUE(history->canUndo()); history->undo();
    EXPECT_FALSE(document->entityByID(added->id()));
    EXPECT_EQ(document->entityByID(locked->id()), locked);
    ASSERT_TRUE(history->canRedo()); history->redo();
    EXPECT_EQ(document->entityByID(added->id()), added);
    EXPECT_EQ(document->layerByName("PARTIAL"), nullptr);
}
