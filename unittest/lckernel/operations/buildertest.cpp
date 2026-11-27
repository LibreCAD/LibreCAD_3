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
    auto nested = std::make_shared<lc::operation::Builder>(document, "Nested");
    edit->appendEntity(locked->move(lc::geo::Coordinate(4, 0))); nested->append(edit); group->append(nested); group->execute();
    EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_TRUE(nested->blockedByLockedLayer()); EXPECT_TRUE(edit->blockedByLockedLayer());
    EXPECT_EQ(document->layerByName("PARTIAL"), nullptr);
    ASSERT_TRUE(history->canUndo()); history->undo();
    EXPECT_FALSE(document->entityByID(added->id()));
    EXPECT_EQ(document->entityByID(locked->id()), locked);
    ASSERT_TRUE(history->canRedo()); history->redo();
    EXPECT_EQ(document->entityByID(added->id()), added);
    EXPECT_EQ(document->layerByName("PARTIAL"), nullptr);
}

TEST(BuilderTest, LockedStackedEditDoesNotPartiallyApplyGroup) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto history = std::make_shared<UndoManagerImpl>(10);
    document->commitProcessEvent().connect<UndoManagerImpl, &UndoManagerImpl::on_CommitProcessEvent>(history.get());
    auto lockedLayer = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1), lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(lockedLayer);
    auto locked = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), lockedLayer);
    document->insertEntity(locked);
    auto freeLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 1), lc::geo::Coordinate(1, 1), document->layerByName("0"));
    auto add = std::make_shared<lc::operation::EntityBuilder>(document); add->appendEntity(freeLine); add->execute();
    history->undo(); ASSERT_TRUE(history->canRedo());
    auto group = std::make_shared<lc::operation::Builder>(document, "Stacked move");
    auto layer = std::make_shared<lc::meta::Layer>("PARTIAL");
    group->append(std::make_shared<lc::operation::AddLayer>(document, layer));
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(locked);
    edit->appendOperation(std::make_shared<lc::operation::Push>());
    edit->appendOperation(std::make_shared<lc::operation::Move>(lc::geo::Coordinate(4, 0)));
    group->append(edit); group->execute();
    EXPECT_TRUE(group->blockedByLockedLayer());
    EXPECT_TRUE(edit->blockedByLockedLayer());
    EXPECT_EQ(document->layerByName("PARTIAL"), nullptr);
    EXPECT_EQ(document->entityByID(locked->id()), locked);
    EXPECT_TRUE(history->canRedo()); EXPECT_FALSE(history->canUndo());
    edit->setAllowLockedLayerEdits(true); group->execute();
    EXPECT_FALSE(group->blockedByLockedLayer());
    EXPECT_EQ(document->layerByName("PARTIAL"), layer);
    const auto moved = document->entityByID(locked->id()); ASSERT_TRUE(moved); EXPECT_NE(moved, locked);
    group->undo(); EXPECT_EQ(document->entityByID(locked->id()), locked);
    EXPECT_EQ(document->layerByName("PARTIAL"), nullptr);
    group->redo(); EXPECT_EQ(document->entityByID(locked->id()), moved);
}

TEST(BuilderTest, LockedCopyAndFilteredUnlockedEditRemainAllowed) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto lockedLayer = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1), lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(lockedLayer);
    auto freeLayer = document->layerByName("0");
    auto locked = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), lockedLayer);
    auto freeLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 1), lc::geo::Coordinate(1, 1), freeLayer);
    document->insertEntity(locked); document->insertEntity(freeLine);
    auto copyGroup = std::make_shared<lc::operation::Builder>(document, "Copy");
    auto copy = std::make_shared<lc::operation::EntityBuilder>(document);
    copy->appendEntity(locked); copy->appendOperation(std::make_shared<lc::operation::Push>());
    copy->appendOperation(std::make_shared<lc::operation::Copy>(lc::geo::Coordinate(2, 0)));
    copyGroup->append(copy); copyGroup->execute();
    EXPECT_FALSE(copyGroup->blockedByLockedLayer());
    EXPECT_EQ(document->entityByID(locked->id()), locked);
    EXPECT_EQ(document->entityContainer().asVector().size(), 3u);
    copyGroup->undo(); EXPECT_EQ(document->entityContainer().asVector().size(), 2u);
    copyGroup->redo(); EXPECT_EQ(document->entityContainer().asVector().size(), 3u);
    copyGroup->undo();

    auto group = std::make_shared<lc::operation::Builder>(document, "Filtered move");
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(locked); edit->appendEntity(freeLine);
    edit->appendOperation(std::make_shared<lc::operation::SelectByLayer>(freeLayer));
    edit->appendOperation(std::make_shared<lc::operation::Move>(lc::geo::Coordinate(4, 0)));
    group->append(edit); group->execute();
    EXPECT_FALSE(group->blockedByLockedLayer());
    EXPECT_EQ(document->entityByID(locked->id()), locked);
    EXPECT_NE(document->entityByID(freeLine->id()), freeLine);
    group->undo(); EXPECT_EQ(document->entityByID(freeLine->id()), freeLine);
}

TEST(BuilderTest, UnknownLockedStackIsNotRunDuringPreflight) {
    struct Custom final : lc::operation::Base {
        explicit Custom(int &calls) : calls(calls) {}
        std::vector<lc::entity::CADEntity_CSPtr> process(
            const std::shared_ptr<lc::storage::Document>,
            const std::vector<lc::entity::CADEntity_CSPtr> entities,
            std::vector<lc::entity::CADEntity_CSPtr>&,
            std::vector<lc::entity::CADEntity_CSPtr>&,
            const std::vector<lc::operation::Base_SPtr>) override {
            ++calls; return entities;
        }
        int &calls;
    };
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1), lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(layer);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), layer);
    document->insertEntity(line);
    int calls = 0;
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(line); edit->appendOperation(std::make_shared<lc::operation::Push>());
    edit->appendOperation(std::make_shared<Custom>(calls));
    auto group = std::make_shared<lc::operation::Builder>(document, "Custom"); group->append(edit); group->execute();
    EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_EQ(calls, 0);
    EXPECT_EQ(document->entityByID(line->id()), line);
    edit->setAllowLockedLayerEdits(true); group->execute();
    EXPECT_FALSE(group->blockedByLockedLayer()); EXPECT_EQ(calls, 1);
}

TEST(BuilderTest, LockedRemoveRotateAndScaleStacksRefuseAndRetry) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1), lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(layer);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(1, 0), lc::geo::Coordinate(2, 0), layer);
    document->insertEntity(line);
    const std::vector<lc::operation::Base_SPtr> operations = {
        std::make_shared<lc::operation::Remove>(),
        std::make_shared<lc::operation::Rotate>(lc::geo::Coordinate(0, 0), 0.5),
        std::make_shared<lc::operation::Scale>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(2, 2))};
    for (const auto &operation : operations) {
        auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
        edit->appendEntity(line); edit->appendOperation(std::make_shared<lc::operation::Push>()); edit->appendOperation(operation);
        auto group = std::make_shared<lc::operation::Builder>(document, "Locked stack");
        group->append(edit); group->execute();
        EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_EQ(document->entityByID(line->id()), line);
        edit->setAllowLockedLayerEdits(true); group->execute();
        EXPECT_FALSE(group->blockedByLockedLayer()); EXPECT_NE(document->entityByID(line->id()), line);
        group->undo(); EXPECT_EQ(document->entityByID(line->id()), line);
    }
}

TEST(BuilderTest, LockedLoopPreflightDoesNotChangeOriginalBeginState) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1), lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(layer);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), layer);
    document->insertEntity(line);
    auto begin = std::make_shared<lc::operation::Begin>();
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(line); edit->appendOperation(std::make_shared<lc::operation::Push>());
    edit->appendOperation(begin); edit->appendOperation(std::make_shared<lc::operation::Move>(lc::geo::Coordinate(1, 0)));
    edit->appendOperation(begin); // the same stateful object appears twice
    edit->appendOperation(std::make_shared<lc::operation::Loop>(2));
    auto group = std::make_shared<lc::operation::Builder>(document, "Locked loop"); group->append(edit); group->execute();
    EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_TRUE(begin->getEntities().empty());
    EXPECT_EQ(document->entityByID(line->id()), line);
    edit->setAllowLockedLayerEdits(true); group->execute();
    EXPECT_FALSE(group->blockedByLockedLayer()); EXPECT_FALSE(begin->getEntities().empty());
    EXPECT_NE(document->entityByID(line->id()), line);
    group->undo(); EXPECT_EQ(document->entityByID(line->id()), line);
}

TEST(BuilderTest, LockedPreflightDoesNotCallEntityTransformsOrAllocateIds) {
    struct SideEffectLine final : lc::entity::Line {
        SideEffectLine(lc::meta::Layer_CSPtr layer, int &calls)
            : Line(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), std::move(layer)), calls(calls) {}
        lc::entity::CADEntity_CSPtr move(const lc::geo::Coordinate &offset) const override {
            ++calls; return Line::move(offset);
        }
        lc::entity::CADEntity_CSPtr copy(const lc::geo::Coordinate &offset) const override {
            ++calls; return Line::copy(offset);
        }
        int &calls;
    };
    for (bool copy : {false, true}) {
        auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
        auto layer = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1), lc::Color(), nullptr, false, false, true);
        document->addDocumentMetaType(layer);
        int calls = 0;
        auto line = std::make_shared<SideEffectLine>(layer, calls); document->insertEntity(line);
        auto edit = std::make_shared<lc::operation::EntityBuilder>(document); edit->appendEntity(line);
        edit->appendOperation(std::make_shared<lc::operation::Push>());
        if (copy) edit->appendOperation(std::make_shared<lc::operation::Copy>(lc::geo::Coordinate(1, 0)));
        else edit->appendOperation(std::make_shared<lc::operation::Move>(lc::geo::Coordinate(1, 0)));
        auto group = std::make_shared<lc::operation::Builder>(document, "Custom entity"); group->append(edit);
        auto before = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), layer);
        group->execute();
        auto after = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), layer);
        EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_EQ(calls, 0);
        EXPECT_EQ(after->id(), before->id() + 1);
        EXPECT_EQ(document->entityByID(line->id()), line);
        edit->setAllowLockedLayerEdits(true); group->execute();
        EXPECT_FALSE(group->blockedByLockedLayer()); EXPECT_EQ(calls, 1);
    }
}

TEST(BuilderTest, NestedLockedEditPreservesParentsAndUndoHistory) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto history = std::make_shared<UndoManagerImpl>(10);
    document->commitProcessEvent().connect<UndoManagerImpl, &UndoManagerImpl::on_CommitProcessEvent>(history.get());
    auto lockedLayer = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1), lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(lockedLayer);
    auto locked = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), lockedLayer);
    document->insertEntity(locked);
    auto freeLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 1), lc::geo::Coordinate(1, 1), document->layerByName("0"));
    auto add = std::make_shared<lc::operation::EntityBuilder>(document); add->appendEntity(freeLine); add->execute();
    history->undo(); ASSERT_TRUE(history->canRedo());
    auto newLayer = std::make_shared<lc::meta::Layer>("OUTER");
    auto block = std::make_shared<lc::meta::Block>("INNER");
    auto outer = std::make_shared<lc::operation::Builder>(document, "Outer");
    auto inner = std::make_shared<lc::operation::Builder>(document, "Inner");
    auto deepest = std::make_shared<lc::operation::Builder>(document, "Deepest");
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(locked); edit->appendOperation(std::make_shared<lc::operation::Push>());
    edit->appendOperation(std::make_shared<lc::operation::Move>(lc::geo::Coordinate(4, 0)));
    deepest->append(edit);
    inner->append(std::make_shared<lc::operation::AddBlock>(document, block)); inner->append(deepest);
    outer->append(std::make_shared<lc::operation::AddLayer>(document, newLayer)); outer->append(inner);
    outer->execute();
    EXPECT_TRUE(outer->blockedByLockedLayer()); EXPECT_TRUE(inner->blockedByLockedLayer());
    EXPECT_TRUE(deepest->blockedByLockedLayer()); EXPECT_TRUE(edit->blockedByLockedLayer());
    EXPECT_FALSE(document->layerByName("OUTER")); EXPECT_FALSE(document->blockByName("INNER"));
    EXPECT_EQ(document->entityByID(locked->id()), locked);
    EXPECT_TRUE(history->canRedo()); EXPECT_FALSE(history->canUndo());
    if (!outer->blockedByLockedLayer()) return;
    edit->setAllowLockedLayerEdits(true); outer->execute();
    EXPECT_FALSE(outer->blockedByLockedLayer()); EXPECT_FALSE(inner->blockedByLockedLayer());
    EXPECT_EQ(document->layerByName("OUTER"), newLayer); EXPECT_EQ(document->blockByName("INNER"), block);
    auto moved = document->entityByID(locked->id()); ASSERT_TRUE(moved); EXPECT_NE(moved, locked);
    outer->undo(); EXPECT_FALSE(document->layerByName("OUTER")); EXPECT_FALSE(document->blockByName("INNER"));
    EXPECT_EQ(document->entityByID(locked->id()), locked);
    outer->redo(); EXPECT_EQ(document->entityByID(locked->id()), moved);
}

TEST(BuilderTest, RejectsCyclicAndNullOperationGroups) {
    EXPECT_THROW(std::make_shared<lc::operation::Builder>(nullptr, "Invalid"), std::invalid_argument);
    EXPECT_THROW(std::make_shared<lc::operation::EntityBuilder>(nullptr), std::invalid_argument);
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto outer = std::make_shared<lc::operation::Builder>(document, "Outer");
    auto inner = std::make_shared<lc::operation::Builder>(document, "Inner");
    EXPECT_THROW(outer->append(nullptr), std::invalid_argument);
    EXPECT_THROW(outer->append(outer), std::invalid_argument);
    outer->append(inner);
    EXPECT_THROW(inner->append(outer), std::invalid_argument);
    // Reusing a child without introducing a cycle is allowed.
    outer->append(inner);
}

TEST(BuilderTest, LockThenNestedEditRefusesBeforeChangingDocument) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto history = std::make_shared<UndoManagerImpl>(10);
    document->commitProcessEvent().connect<UndoManagerImpl, &UndoManagerImpl::on_CommitProcessEvent>(history.get());
    auto originalLayer = document->layerByName("0");
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), originalLayer);
    document->insertEntity(line);
    auto temp = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 1), lc::geo::Coordinate(1, 1), originalLayer);
    auto prior = std::make_shared<lc::operation::EntityBuilder>(document); prior->appendEntity(temp); prior->execute();
    history->undo(); ASSERT_TRUE(history->canRedo());
    auto lockedLayer = lc::meta::Layer::withState(originalLayer, false, false, true);
    auto outer = std::make_shared<lc::operation::Builder>(document, "Lock and edit");
    auto inner = std::make_shared<lc::operation::Builder>(document, "Nested edit");
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(line); edit->appendOperation(std::make_shared<lc::operation::Push>());
    edit->appendOperation(std::make_shared<lc::operation::Move>(lc::geo::Coordinate(5, 0)));
    inner->append(edit);
    outer->append(std::make_shared<lc::operation::ReplaceLayer>(document, originalLayer, lockedLayer));
    outer->append(inner); outer->execute();
    EXPECT_TRUE(outer->blockedByLockedLayer()); EXPECT_TRUE(inner->blockedByLockedLayer());
    EXPECT_TRUE(edit->blockedByLockedLayer()); EXPECT_EQ(document->layerByName("0"), originalLayer);
    EXPECT_EQ(document->entityByID(line->id()), line); EXPECT_TRUE(history->canRedo());
    if (!outer->blockedByLockedLayer()) return;
    edit->setAllowLockedLayerEdits(true); outer->execute();
    EXPECT_FALSE(outer->blockedByLockedLayer()); EXPECT_TRUE(document->layerByName("0")->isLocked());
    EXPECT_NE(document->entityByID(line->id()), line);
    outer->undo(); EXPECT_EQ(document->layerByName("0"), originalLayer);
    auto restored = std::dynamic_pointer_cast<const lc::entity::Line>(document->entityByID(line->id()));
    ASSERT_TRUE(restored); EXPECT_EQ(restored->layer(), originalLayer);
    EXPECT_DOUBLE_EQ(restored->start().x(), line->start().x());
    EXPECT_DOUBLE_EQ(restored->end().x(), line->end().x());
    outer->redo(); EXPECT_TRUE(document->layerByName("0")->isLocked());
}

TEST(BuilderTest, LayerLockPreflightTracksChainedReplacementsAndUnrelatedEdits) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto original = document->layerByName("0");
    auto other = std::make_shared<lc::meta::Layer>("OTHER"); document->addDocumentMetaType(other);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), original);
    auto freeLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 1), lc::geo::Coordinate(1, 1), other);
    document->insertEntity(line); document->insertEntity(freeLine);
    auto renamed = std::make_shared<lc::meta::Layer>("RENAMED");
    auto locked = lc::meta::Layer::withState(renamed, false, false, true);
    auto group = std::make_shared<lc::operation::Builder>(document, "Chained lock");
    auto inner = std::make_shared<lc::operation::Builder>(document, "Rename");
    inner->append(std::make_shared<lc::operation::ReplaceLayer>(document, original, renamed));
    group->append(inner);
    group->append(std::make_shared<lc::operation::ReplaceLayer>(document, renamed, locked));
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(line->move(lc::geo::Coordinate(2, 0))); group->append(edit); group->execute();
    EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_TRUE(edit->blockedByLockedLayer());
    EXPECT_EQ(document->entityByID(line->id()), line); EXPECT_EQ(document->layerByName("0"), original);

    auto unrelated = std::make_shared<lc::operation::Builder>(document, "Unrelated edit");
    auto freeEdit = std::make_shared<lc::operation::EntityBuilder>(document);
    auto moved = freeLine->move(lc::geo::Coordinate(3, 0)); freeEdit->appendEntity(moved);
    unrelated->append(std::make_shared<lc::operation::ReplaceLayer>(document, original, lc::meta::Layer::withState(original, false, false, true)));
    unrelated->append(freeEdit); unrelated->execute();
    EXPECT_FALSE(unrelated->blockedByLockedLayer()); EXPECT_EQ(document->entityByID(freeLine->id()), moved);
    unrelated->undo(); EXPECT_EQ(document->entityByID(freeLine->id()), freeLine);
}
