#include <gtest/gtest.h>
#include <cmath>
#include <cad/operations/builder.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/operations/layerops.h>
#include <cad/operations/blockops.h>
#include <cad/operations/entitybuilder.h>
#include <cad/operations/entityops.h>
#include <cad/storage/undomanagerimpl.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/image.h>
#include <cad/primitive/point.h>
#include <cad/primitive/spline.h>

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

TEST(BuilderTest, CreatedEntityLockedLaterInGroupIsRejectedAtomicallyAndRetryable) {
    for (int variant = 0; variant != 4; ++variant) {
        const bool nested = (variant & 1) != 0;
        const bool viaStack = (variant & 2) != 0;
        SCOPED_TRACE(nested ? "nested" : "flat");
        SCOPED_TRACE(viaStack ? "stack" : "direct");
        auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
        auto history = std::make_shared<UndoManagerImpl>(10);
        document->commitProcessEvent().connect<UndoManagerImpl, &UndoManagerImpl::on_CommitProcessEvent>(history.get());
        auto layer = document->layerByName("0");
        auto prior = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), layer);
        auto priorAdd = std::make_shared<lc::operation::EntityBuilder>(document);
        priorAdd->appendEntity(prior); priorAdd->execute();
        history->undo(); ASSERT_TRUE(history->canRedo()); ASSERT_FALSE(history->canUndo());
        auto locked = lc::meta::Layer::withState(layer, false, false, true);
        auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(1, 2, 3),
                                                        lc::geo::Coordinate(4, 5, 6), layer);
        auto add = std::make_shared<lc::operation::EntityBuilder>(document);
        add->appendEntity(line);
        auto lock = std::make_shared<lc::operation::ReplaceLayer>(document, layer, locked);
        auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
        if (viaStack) {
            edit->appendEntity(line);
            edit->appendOperation(std::make_shared<lc::operation::Push>());
            edit->appendOperation(std::make_shared<lc::operation::Move>(lc::geo::Coordinate(10, 0, 0)));
        } else {
            edit->appendEntity(line->move(lc::geo::Coordinate(10, 0, 0)));
        }
        auto group = std::make_shared<lc::operation::Builder>(document, "Create lock and edit");
        group->append(add);
        if (nested) {
            auto child = std::make_shared<lc::operation::Builder>(document, "Lock and edit");
            child->append(lock); child->append(edit); group->append(child);
        } else {
            group->append(lock); group->append(edit);
        }
        group->execute();
        EXPECT_TRUE(group->blockedByLockedLayer());
        EXPECT_TRUE(edit->blockedByLockedLayer());
        EXPECT_EQ(document->entityByID(line->id()), nullptr);
        EXPECT_EQ(document->layerByName("0"), layer);
        EXPECT_TRUE(history->canRedo()); EXPECT_FALSE(history->canUndo());
        group->undo(); group->redo();
        EXPECT_EQ(document->entityByID(line->id()), nullptr);
        EXPECT_EQ(document->layerByName("0"), layer);

        edit->setAllowLockedLayerEdits(true);
        group->execute();
        EXPECT_FALSE(group->blockedByLockedLayer());
        auto moved = std::dynamic_pointer_cast<const lc::entity::Line>(document->entityByID(line->id()));
        ASSERT_TRUE(moved);
        EXPECT_EQ(moved->layer(), locked);
        EXPECT_DOUBLE_EQ(moved->start().x(), 11);
        EXPECT_DOUBLE_EQ(moved->start().z(), 3);
        group->undo();
        EXPECT_EQ(document->entityByID(line->id()), nullptr);
        EXPECT_EQ(document->layerByName("0"), layer);
        group->redo();
        EXPECT_EQ(document->entityByID(line->id()), moved);
        EXPECT_EQ(document->layerByName("0"), locked);
    }
}

TEST(BuilderTest, GroupMayAddAndLeaveNewLockedEntityUnchanged) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto locked = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1),
                                                    lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(locked);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
                                                  lc::geo::Coordinate(1, 0), locked);
    auto group = std::make_shared<lc::operation::Builder>(document, "Insert locked line");
    auto add = std::make_shared<lc::operation::EntityBuilder>(document);
    add->appendEntity(line); group->append(add);
    auto unchanged = std::make_shared<lc::operation::EntityBuilder>(document);
    unchanged->appendEntity(line); group->append(unchanged);
    group->execute();
    EXPECT_FALSE(group->blockedByLockedLayer());
    EXPECT_EQ(document->entityByID(line->id()), line);
    group->undo(); EXPECT_EQ(document->entityByID(line->id()), nullptr);
    group->redo(); EXPECT_EQ(document->entityByID(line->id()), line);
}

TEST(BuilderTest, GroupRejectsEditOfNewEntityOnAlreadyLockedLayer) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto locked = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1),
                                                    lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(locked);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
                                                  lc::geo::Coordinate(1, 0), locked);
    auto group = std::make_shared<lc::operation::Builder>(document, "Insert and edit locked line");
    auto add = std::make_shared<lc::operation::EntityBuilder>(document);
    add->appendEntity(line); group->append(add);
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(line->move(lc::geo::Coordinate(3, 0))); group->append(edit);
    group->execute();
    EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_TRUE(edit->blockedByLockedLayer());
    EXPECT_EQ(document->entityByID(line->id()), nullptr);
    edit->setAllowLockedLayerEdits(true); group->execute();
    EXPECT_FALSE(group->blockedByLockedLayer());
    auto moved = std::dynamic_pointer_cast<const lc::entity::Line>(document->entityByID(line->id()));
    ASSERT_TRUE(moved); EXPECT_DOUBLE_EQ(moved->start().x(), 3);
    group->undo(); EXPECT_EQ(document->entityByID(line->id()), nullptr);
    group->redo(); EXPECT_EQ(document->entityByID(line->id()), moved);
}

TEST(BuilderTest, GroupTracksStagedLayersAtTheirActualInsertionTime) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto locked = lc::meta::Layer::withState(document->layerByName("0"), false, false, true);
    auto unlocked = lc::meta::Layer::withState(locked, false, false, false);
    document->removeDocumentMetaType(document->layerByName("0"));
    document->addDocumentMetaType(locked);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
                                                  lc::geo::Coordinate(1, 0), locked);
    auto group = std::make_shared<lc::operation::Builder>(document, "Unlock then insert stale layer");
    group->append(std::make_shared<lc::operation::ReplaceLayer>(document, locked, unlocked));
    auto add = std::make_shared<lc::operation::EntityBuilder>(document);
    add->appendEntity(line); group->append(add);
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(line->move(lc::geo::Coordinate(3, 0))); group->append(edit);
    group->execute();
    EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_TRUE(edit->blockedByLockedLayer());
    EXPECT_EQ(document->entityByID(line->id()), nullptr);
    EXPECT_EQ(document->layerByName("0"), locked);
}

TEST(BuilderTest, GroupTracksEarlierForcedStackEditOfNewLockedEntity) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto locked = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1),
                                                    lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(locked);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
                                                  lc::geo::Coordinate(1, 0), locked);
    auto group = std::make_shared<lc::operation::Builder>(document, "Insert and edit twice");
    auto add = std::make_shared<lc::operation::EntityBuilder>(document);
    add->appendEntity(line); group->append(add);
    auto forced = std::make_shared<lc::operation::EntityBuilder>(document);
    forced->setAllowLockedLayerEdits(true); forced->appendEntity(line);
    forced->appendOperation(std::make_shared<lc::operation::Push>());
    forced->appendOperation(std::make_shared<lc::operation::Move>(lc::geo::Coordinate(1, 0)));
    group->append(forced);
    auto stale = std::make_shared<lc::operation::EntityBuilder>(document);
    stale->appendEntity(line); group->append(stale);
    group->execute();
    EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_TRUE(stale->blockedByLockedLayer());
    EXPECT_EQ(document->entityByID(line->id()), nullptr);
}

TEST(BuilderTest, SingleOperationRejectsRepeatedNewIdentityOnLockedLayer) {
    for (bool grouped : {false, true}) {
        SCOPED_TRACE(grouped ? "grouped" : "direct");
        auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
        auto locked = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1),
                                                        lc::Color(), nullptr, false, false, true);
        document->addDocumentMetaType(locked);
        auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
                                                      lc::geo::Coordinate(1, 0), locked);
        auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
        edit->appendEntity(line);
        edit->appendEntity(line->move(lc::geo::Coordinate(3, 0)));
        auto group = std::make_shared<lc::operation::Builder>(document, "Insert duplicate identity");
        if (grouped) { group->append(edit); group->execute(); } else edit->execute();
        EXPECT_TRUE(edit->blockedByLockedLayer());
        if (grouped) EXPECT_TRUE(group->blockedByLockedLayer());
        EXPECT_EQ(document->entityByID(line->id()), nullptr);
        edit->setAllowLockedLayerEdits(true);
        if (grouped) group->execute(); else edit->execute();
        auto moved = std::dynamic_pointer_cast<const lc::entity::Line>(document->entityByID(line->id()));
        ASSERT_TRUE(moved); EXPECT_DOUBLE_EQ(moved->start().x(), 3);
    }
}

TEST(BuilderTest, SingleOperationMayUpdateNewIdentityOnUnlockedLayer) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = document->layerByName("0");
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
                                                  lc::geo::Coordinate(1, 0), layer);
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(line); edit->appendEntity(line->move(lc::geo::Coordinate(3, 0)));
    edit->execute();
    EXPECT_FALSE(edit->blockedByLockedLayer());
    auto moved = std::dynamic_pointer_cast<const lc::entity::Line>(document->entityByID(line->id()));
    ASSERT_TRUE(moved); EXPECT_DOUBLE_EQ(moved->start().x(), 3);
}

TEST(BuilderTest, GroupMayEditEntityCreatedEarlierOnUnlockedLayer) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = document->layerByName("0");
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
                                                  lc::geo::Coordinate(1, 0), layer);
    auto group = std::make_shared<lc::operation::Builder>(document, "Create and edit");
    auto add = std::make_shared<lc::operation::EntityBuilder>(document);
    add->appendEntity(line); group->append(add);
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(line->move(lc::geo::Coordinate(3, 0))); group->append(edit);
    group->execute();
    EXPECT_FALSE(group->blockedByLockedLayer());
    auto moved = std::dynamic_pointer_cast<const lc::entity::Line>(document->entityByID(line->id()));
    ASSERT_TRUE(moved); EXPECT_DOUBLE_EQ(moved->start().x(), 3);
    group->undo(); EXPECT_EQ(document->entityByID(line->id()), nullptr);
    group->redo(); EXPECT_EQ(document->entityByID(line->id()), moved);
}

TEST(BuilderTest, GroupPreflightsNewEntityProducedByBuiltInStack) {
    for (int kind = 0; kind != 7; ++kind) for (int variant = 0; variant != 18; ++variant) {
        const bool nested = (variant & 1) != 0;
        const int mode = variant / 2;
        SCOPED_TRACE(kind);
        SCOPED_TRACE(nested ? "nested" : "flat");
        SCOPED_TRACE(mode);
        auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
        auto layer = document->layerByName("0");
        auto locked = lc::meta::Layer::withState(layer, false, false, true);
        lc::entity::CADEntity_CSPtr line;
        if (kind == 0) line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(1, 2, 3),
                                                                  lc::geo::Coordinate(4, 5, 6), layer);
        else if (kind == 1) line = std::make_shared<lc::entity::Circle>(lc::geo::Coordinate(1, 2, 3), 4, layer);
        else if (kind == 2) line = std::make_shared<lc::entity::Arc>(lc::geo::Coordinate(1, 2, 3), 4, 0.25, 1.5, true, layer);
        else if (kind == 3) line = std::make_shared<lc::entity::Ellipse>(lc::geo::Coordinate(1, 2, 3),
                                                                          lc::geo::Coordinate(4, 2), 2, 0.25, 1.5, true, layer);
        else if (kind == 4) line = std::make_shared<lc::entity::Spline>(
            std::vector<lc::geo::Coordinate>{{1, 2, 3}, {4, 5, 3}, {7, 8, 3}},
            std::vector<double>{0, 0, 0, 1, 1, 1}, std::vector<lc::geo::Coordinate>{},
            2, false, 0.01, 1, 2, 3, 4, 5, 6, 0, 0, 1,
            lc::geo::Spline::RATIONAL, layer, nullptr, nullptr, std::vector<double>{1, 0.5, 1});
        else if (kind == 5) line = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(1, 2, 3), layer);
        else line = std::make_shared<lc::entity::Image>("image.png", lc::geo::Coordinate(1, 2, 3),
            lc::geo::Coordinate(1, 0, 1), lc::geo::Coordinate(0, 1, -1), 2, 3, 40, 60, 10, layer, nullptr);
        auto create = std::make_shared<lc::operation::EntityBuilder>(document);
        create->appendEntity(line);
        if (mode == 5) create->appendOperation(std::make_shared<lc::operation::SelectByLayer>(layer));
        else create->appendOperation(std::make_shared<lc::operation::Push>());
        if (mode == 8) create->appendOperation(std::make_shared<lc::operation::Begin>());
        if (mode == 1 || mode == 7 || mode == 8) {
            create->appendOperation(std::make_shared<lc::operation::Copy>(lc::geo::Coordinate(2, 0)));
            if (mode == 7) create->appendOperation(std::make_shared<lc::operation::Remove>());
            if (mode == 8) create->appendOperation(std::make_shared<lc::operation::Loop>(2));
        }
        else if (mode == 2) create->appendOperation(std::make_shared<lc::operation::Rotate>(lc::geo::Coordinate(), 0.5));
        else if (mode == 3) create->appendOperation(std::make_shared<lc::operation::Scale>(lc::geo::Coordinate(), lc::geo::Coordinate(2, 3, 1)));
        else {
            if (mode == 4 || mode == 6) create->appendOperation(std::make_shared<lc::operation::Begin>());
            create->appendOperation(std::make_shared<lc::operation::Move>(lc::geo::Coordinate(2, 0)));
            if (mode == 6) create->appendOperation(std::make_shared<lc::operation::Loop>(2));
        }
        auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
        edit->appendEntity(line->move(lc::geo::Coordinate(10, 0)));
        auto group = std::make_shared<lc::operation::Builder>(document, "Stack create then lock");
        group->append(create);
        auto lock = std::make_shared<lc::operation::ReplaceLayer>(document, layer, locked);
        if (nested) {
            auto child = std::make_shared<lc::operation::Builder>(document, "Lock and edit");
            child->append(lock); child->append(edit); group->append(child);
        } else { group->append(lock); group->append(edit); }
        const auto before = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), layer);
        group->execute();
        const auto after = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), layer);
        EXPECT_EQ(after->id(), before->id() + 1); // shadow projection allocates no entity IDs
        EXPECT_TRUE(group->blockedByLockedLayer()); EXPECT_TRUE(edit->blockedByLockedLayer());
        EXPECT_EQ(document->entityByID(line->id()), nullptr);
        EXPECT_EQ(document->layerByName("0"), layer);
        edit->setAllowLockedLayerEdits(true); group->execute();
        EXPECT_FALSE(group->blockedByLockedLayer());
        const auto moved = document->entityByID(line->id());
        ASSERT_TRUE(moved); EXPECT_EQ(moved->layer(), locked);
        if (kind == 0) { auto value = std::dynamic_pointer_cast<const lc::entity::Line>(moved); ASSERT_TRUE(value); EXPECT_DOUBLE_EQ(value->start().x(), 11); }
        if (kind == 1) { auto value = std::dynamic_pointer_cast<const lc::entity::Circle>(moved); ASSERT_TRUE(value); EXPECT_DOUBLE_EQ(value->center().x(), 11); }
        if (kind == 2) { auto value = std::dynamic_pointer_cast<const lc::entity::Arc>(moved); ASSERT_TRUE(value); EXPECT_DOUBLE_EQ(value->center().x(), 11); }
        if (kind == 3) { auto value = std::dynamic_pointer_cast<const lc::entity::Ellipse>(moved); ASSERT_TRUE(value); EXPECT_DOUBLE_EQ(value->center().x(), 11); }
        if (kind == 4) { auto value = std::dynamic_pointer_cast<const lc::entity::Spline>(moved); ASSERT_TRUE(value); EXPECT_DOUBLE_EQ(value->controlPoints().at(0).x(), 11); }
        if (kind == 5) { auto value = std::dynamic_pointer_cast<const lc::entity::Point>(moved); ASSERT_TRUE(value); EXPECT_DOUBLE_EQ(value->x(), 11); EXPECT_DOUBLE_EQ(value->z(), 3); }
        if (kind == 6) { auto value = std::dynamic_pointer_cast<const lc::entity::Image>(moved); ASSERT_TRUE(value); EXPECT_DOUBLE_EQ(value->base().x(), 11); EXPECT_DOUBLE_EQ(value->base().z(), 3); }
        const auto expectedCount = mode == 1 ? 2u : mode == 8 ? 4u : 1u;
        EXPECT_EQ(document->entityContainer().asVector().size(), expectedCount);
        group->undo(); EXPECT_EQ(document->entityByID(line->id()), nullptr);
        EXPECT_TRUE(document->entityContainer().asVector().empty());
        group->redo(); EXPECT_EQ(document->entityByID(line->id()), moved);
        EXPECT_EQ(document->entityContainer().asVector().size(), expectedCount);
    }
}

TEST(BuilderTest, GroupMayReinsertNewEntityAfterForcedLockedRemoval) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto locked = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1),
                                                    lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(locked);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
                                                  lc::geo::Coordinate(1, 0), locked);
    auto add = std::make_shared<lc::operation::EntityBuilder>(document);
    add->appendEntity(line);
    auto remove = std::make_shared<lc::operation::EntityBuilder>(document);
    remove->setAllowLockedLayerEdits(true); remove->appendEntity(line);
    remove->appendOperation(std::make_shared<lc::operation::Push>());
    remove->appendOperation(std::make_shared<lc::operation::Remove>());
    auto reinsert = std::make_shared<lc::operation::EntityBuilder>(document);
    reinsert->appendEntity(line);
    auto group = std::make_shared<lc::operation::Builder>(document, "Create remove reinsert");
    group->append(add); group->append(remove); group->append(reinsert);
    group->execute(); EXPECT_FALSE(group->blockedByLockedLayer());
    EXPECT_EQ(document->entityByID(line->id()), line);
    group->undo(); EXPECT_EQ(document->entityByID(line->id()), nullptr);
    group->redo(); EXPECT_EQ(document->entityByID(line->id()), line);
}

TEST(BuilderTest, GroupMayInsertAfterRemovingAnAbsentEntity) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto locked = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1),
                                                    lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(locked);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
                                                  lc::geo::Coordinate(1, 0), locked);
    auto remove = std::make_shared<lc::operation::EntityBuilder>(document);
    remove->appendEntity(line);
    remove->appendOperation(std::make_shared<lc::operation::Push>());
    remove->appendOperation(std::make_shared<lc::operation::Remove>());
    auto add = std::make_shared<lc::operation::EntityBuilder>(document);
    add->appendEntity(line);
    auto group = std::make_shared<lc::operation::Builder>(document, "Remove absent then insert");
    group->append(remove); group->append(add);
    group->execute(); EXPECT_FALSE(group->blockedByLockedLayer());
    EXPECT_EQ(document->entityByID(line->id()), line);
    group->undo(); EXPECT_EQ(document->entityByID(line->id()), nullptr);
    group->redo(); EXPECT_EQ(document->entityByID(line->id()), line);
}

TEST(BuilderTest, GroupDoesNotProjectStackInputThatIsNotEmitted) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto locked = std::make_shared<lc::meta::Layer>("LOCKED", lc::meta::MetaLineWidthByValue(1),
                                                    lc::Color(), nullptr, false, false, true);
    document->addDocumentMetaType(locked);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0),
                                                  lc::geo::Coordinate(1, 0), locked);
    auto dropped = std::make_shared<lc::operation::EntityBuilder>(document);
    dropped->appendEntity(line);
    dropped->appendOperation(std::make_shared<lc::operation::Push>());
    // SelectByLayer consumes the working buffer, which Push already cleared.
    dropped->appendOperation(std::make_shared<lc::operation::SelectByLayer>(locked));
    auto add = std::make_shared<lc::operation::EntityBuilder>(document);
    add->appendEntity(line);
    auto group = std::make_shared<lc::operation::Builder>(document, "Drop then insert");
    group->append(dropped); group->append(add); group->execute();
    EXPECT_FALSE(group->blockedByLockedLayer());
    EXPECT_EQ(document->entityByID(line->id()), line);
    group->undo(); EXPECT_EQ(document->entityByID(line->id()), nullptr);
    group->redo(); EXPECT_EQ(document->entityByID(line->id()), line);
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
    auto moved = std::dynamic_pointer_cast<const lc::entity::Line>(document->entityByID(line->id()));
    ASSERT_TRUE(moved); EXPECT_NE(moved, line); EXPECT_EQ(moved->layer(), lockedLayer);
    EXPECT_DOUBLE_EQ(moved->start().x(), 5.0);
    outer->undo(); EXPECT_EQ(document->layerByName("0"), originalLayer);
    auto restored = std::dynamic_pointer_cast<const lc::entity::Line>(document->entityByID(line->id()));
    ASSERT_TRUE(restored); EXPECT_EQ(restored->layer(), originalLayer);
    EXPECT_DOUBLE_EQ(restored->start().x(), line->start().x());
    EXPECT_DOUBLE_EQ(restored->end().x(), line->end().x());
    outer->redo(); EXPECT_TRUE(document->layerByName("0")->isLocked());
    EXPECT_EQ(document->entityByID(line->id())->layer(), lockedLayer);
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

TEST(BuilderTest, ForcedLineEditPreservesIntentionalDifferentLayer) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto originalLayer = document->layerByName("0");
    auto target = std::make_shared<lc::meta::Layer>("TARGET"); document->addDocumentMetaType(target);
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 0), originalLayer);
    document->insertEntity(line);
    auto locked = lc::meta::Layer::withState(originalLayer, false, false, true);
    auto lock = std::make_shared<lc::operation::ReplaceLayer>(document, originalLayer, locked); lock->execute();
    auto movedToLayer = line->modify(target, line->metaInfo(), line->block());
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->setAllowLockedLayerEdits(true); edit->appendEntity(movedToLayer); edit->execute();
    EXPECT_EQ(document->entityByID(line->id()), movedToLayer);
    EXPECT_EQ(document->entityByID(line->id())->layer(), target);
    edit->undo(); EXPECT_EQ(document->entityByID(line->id())->layer(), locked);
    edit->redo(); EXPECT_EQ(document->entityByID(line->id())->layer(), target);
}

TEST(BuilderTest, ForcedCurvedEditRetainsCurrentLayerAndGeometry) {
    for (const bool isArc : {false, true}) {
        SCOPED_TRACE(isArc ? "arc" : "circle");
        auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
        auto layer = document->layerByName("0");
        auto meta = lc::meta::MetaInfo::create();
        lc::entity::CADEntity_CSPtr entity;
        if (isArc)
            entity = std::make_shared<lc::entity::Arc>(lc::geo::Coordinate(1, 2, 3), 4, 0.25, 1.5, true, layer, meta);
        else
            entity = std::make_shared<lc::entity::Circle>(lc::geo::Coordinate(1, 2, 3), 4, layer, meta);
        document->insertEntity(entity);
        auto locked = lc::meta::Layer::withState(layer, false, false, true);
        auto lock = std::make_shared<lc::operation::ReplaceLayer>(document, layer, locked);
        lock->execute();
        auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
        edit->appendEntity(entity->move(lc::geo::Coordinate(5, 6)));
        edit->setAllowLockedLayerEdits(true);
        edit->execute();
        const auto verify = [&] {
            auto result = document->entityByID(entity->id());
            ASSERT_TRUE(result);
            EXPECT_EQ(result->layer(), locked);
            EXPECT_EQ(result->metaInfo(), meta);
            if (isArc) {
                auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(result);
                ASSERT_TRUE(arc);
                EXPECT_DOUBLE_EQ(arc->center().x(), 6); EXPECT_DOUBLE_EQ(arc->center().y(), 8);
                EXPECT_DOUBLE_EQ(arc->center().z(), 3);
                EXPECT_DOUBLE_EQ(arc->radius(), 4); EXPECT_DOUBLE_EQ(arc->startAngle(), 0.25);
                EXPECT_DOUBLE_EQ(arc->endAngle(), 1.5); EXPECT_TRUE(arc->CCW());
            } else {
                auto circle = std::dynamic_pointer_cast<const lc::entity::Circle>(result);
                ASSERT_TRUE(circle);
                EXPECT_DOUBLE_EQ(circle->center().x(), 6); EXPECT_DOUBLE_EQ(circle->center().y(), 8);
                EXPECT_DOUBLE_EQ(circle->center().z(), 3);
                EXPECT_DOUBLE_EQ(circle->radius(), 4);
            }
        };
        verify();
        edit->undo(); EXPECT_EQ(document->entityByID(entity->id())->layer(), locked);
        edit->redo(); verify();

        auto target = std::make_shared<lc::meta::Layer>("TARGET");
        document->addDocumentMetaType(target);
        auto moved = entity->modify(target, entity->metaInfo(), entity->block());
        auto changeLayer = std::make_shared<lc::operation::EntityBuilder>(document);
        changeLayer->setAllowLockedLayerEdits(true);
        changeLayer->appendEntity(moved); changeLayer->execute();
        EXPECT_EQ(document->entityByID(entity->id())->layer(), target);
        changeLayer->undo(); verify();
        changeLayer->redo(); EXPECT_EQ(document->entityByID(entity->id())->layer(), target);
    }
}

TEST(BuilderTest, ForcedEllipseAndSplineEditsRetainCurrentLayerAndGeometry) {
    for (bool isSpline : {false, true}) {
        SCOPED_TRACE(isSpline ? "spline" : "ellipse");
        auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
        auto layer = document->layerByName("0");
        auto meta = lc::meta::MetaInfo::create();
        lc::entity::CADEntity_CSPtr entity;
        if (isSpline) {
            entity = std::make_shared<lc::entity::Spline>(
                std::vector<lc::geo::Coordinate>{{1, 2, 3}, {4, 5, 3}, {7, 8, 3}},
                std::vector<double>{0, 0, 0, 1, 1, 1}, std::vector<lc::geo::Coordinate>{},
                2, false, 0.01, 1, 2, 3, 4, 5, 6, 0, 0, 1,
                lc::geo::Spline::RATIONAL, layer, meta, nullptr, std::vector<double>{1, 0.5, 1});
        } else {
            entity = std::make_shared<lc::entity::Ellipse>(lc::geo::Coordinate(1, 2, 3),
                lc::geo::Coordinate(4, 2, 0), 2, 0.25, 1.5, true, layer, meta);
        }
        document->insertEntity(entity);
        auto locked = lc::meta::Layer::withState(layer, false, false, true);
        auto lock = std::make_shared<lc::operation::ReplaceLayer>(document, layer, locked);
        lock->execute();
        auto candidate = entity->move(lc::geo::Coordinate(5, 6, 7));
        auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
        edit->appendEntity(candidate);
        edit->setAllowLockedLayerEdits(true);
        edit->execute();
        const auto verify = [&] {
            auto result = document->entityByID(entity->id());
            ASSERT_TRUE(result);
            EXPECT_EQ(result->layer(), locked);
            EXPECT_EQ(result->metaInfo(), meta);
            if (isSpline) {
                auto expected = std::dynamic_pointer_cast<const lc::entity::Spline>(candidate);
                auto actual = std::dynamic_pointer_cast<const lc::entity::Spline>(result);
                ASSERT_TRUE(expected); ASSERT_TRUE(actual);
                EXPECT_EQ(actual->controlPoints(), expected->controlPoints());
                EXPECT_EQ(actual->fitPoints(), expected->fitPoints());
                EXPECT_EQ(actual->knotPoints(), expected->knotPoints());
                EXPECT_EQ(actual->weights(), expected->weights());
                EXPECT_EQ(actual->flags(), expected->flags());
                EXPECT_EQ(actual->degree(), expected->degree());
                EXPECT_EQ(actual->closed(), expected->closed());
                EXPECT_DOUBLE_EQ(actual->fitTolerance(), expected->fitTolerance());
                EXPECT_DOUBLE_EQ(actual->nX(), expected->nX());
                EXPECT_DOUBLE_EQ(actual->nY(), expected->nY());
                EXPECT_DOUBLE_EQ(actual->nZ(), expected->nZ());
                EXPECT_DOUBLE_EQ(actual->startTanX(), expected->startTanX());
                EXPECT_DOUBLE_EQ(actual->startTanY(), expected->startTanY());
                EXPECT_DOUBLE_EQ(actual->startTanZ(), expected->startTanZ());
                EXPECT_DOUBLE_EQ(actual->endTanX(), expected->endTanX());
                EXPECT_DOUBLE_EQ(actual->endTanY(), expected->endTanY());
                EXPECT_DOUBLE_EQ(actual->endTanZ(), expected->endTanZ());
            } else {
                auto expected = std::dynamic_pointer_cast<const lc::entity::Ellipse>(candidate);
                auto actual = std::dynamic_pointer_cast<const lc::entity::Ellipse>(result);
                ASSERT_TRUE(expected); ASSERT_TRUE(actual);
                EXPECT_EQ(actual->center(), expected->center());
                EXPECT_EQ(actual->majorP(), expected->majorP());
                EXPECT_DOUBLE_EQ(actual->minorRadius(), expected->minorRadius());
                EXPECT_DOUBLE_EQ(actual->startAngle(), expected->startAngle());
                EXPECT_DOUBLE_EQ(actual->endAngle(), expected->endAngle());
                EXPECT_EQ(actual->isReversed(), expected->isReversed());
            }
        };
        verify();
        edit->undo(); EXPECT_EQ(document->entityByID(entity->id())->layer(), locked);
        edit->redo(); verify();

        auto target = std::make_shared<lc::meta::Layer>("TARGET");
        document->addDocumentMetaType(target);
        auto changeLayer = std::make_shared<lc::operation::EntityBuilder>(document);
        changeLayer->setAllowLockedLayerEdits(true);
        changeLayer->appendEntity(entity->modify(target, entity->metaInfo(), entity->block()));
        changeLayer->execute();
        EXPECT_EQ(document->entityByID(entity->id())->layer(), target);
        changeLayer->undo(); verify();
        changeLayer->redo(); EXPECT_EQ(document->entityByID(entity->id())->layer(), target);
    }
}

TEST(BuilderTest, PointLayerReplacementAndTransformsPreserve3dCoordinate) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = document->layerByName("0");
    auto metadata = lc::meta::MetaInfo::create();
    auto point = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(1, 2, 3), layer, metadata);
    document->insertEntity(point);
    auto moved = std::dynamic_pointer_cast<const lc::entity::Point>(point->move(lc::geo::Coordinate(5, 6, 7)));
    auto copied = std::dynamic_pointer_cast<const lc::entity::Point>(point->copy(lc::geo::Coordinate(5, 6, 7)));
    auto rotated = std::dynamic_pointer_cast<const lc::entity::Point>(point->rotate(lc::geo::Coordinate(), std::acos(-1.0) / 2));
    auto scaled = std::dynamic_pointer_cast<const lc::entity::Point>(point->scale(lc::geo::Coordinate(), lc::geo::Coordinate(2, 3, 4)));
    auto mirrored = std::dynamic_pointer_cast<const lc::entity::Point>(point->mirror(lc::geo::Coordinate(), lc::geo::Coordinate(1, 0)));
    ASSERT_TRUE(moved); ASSERT_TRUE(copied); ASSERT_TRUE(rotated); ASSERT_TRUE(scaled); ASSERT_TRUE(mirrored);
    EXPECT_EQ(moved->id(), point->id()); EXPECT_NE(copied->id(), point->id());
    EXPECT_DOUBLE_EQ(moved->x(), 6); EXPECT_DOUBLE_EQ(moved->y(), 8); EXPECT_DOUBLE_EQ(moved->z(), 10);
    EXPECT_DOUBLE_EQ(copied->x(), 6); EXPECT_DOUBLE_EQ(copied->y(), 8); EXPECT_DOUBLE_EQ(copied->z(), 10);
    EXPECT_NEAR(rotated->x(), -2, 1e-12); EXPECT_NEAR(rotated->y(), 1, 1e-12); EXPECT_DOUBLE_EQ(rotated->z(), 3);
    EXPECT_DOUBLE_EQ(scaled->x(), 2); EXPECT_DOUBLE_EQ(scaled->y(), 6); EXPECT_DOUBLE_EQ(scaled->z(), 12);
    EXPECT_DOUBLE_EQ(mirrored->x(), 1); EXPECT_DOUBLE_EQ(mirrored->y(), -2); EXPECT_DOUBLE_EQ(mirrored->z(), -3);
    EXPECT_EQ(mirrored->id(), point->id()); EXPECT_EQ(mirrored->layer(), layer);
    auto shiftedMirror = std::dynamic_pointer_cast<const lc::entity::Point>(point->mirror(
        lc::geo::Coordinate(2, 0, 3), lc::geo::Coordinate(2, 1, 3)));
    ASSERT_TRUE(shiftedMirror); EXPECT_DOUBLE_EQ(shiftedMirror->x(), 3);
    EXPECT_DOUBLE_EQ(shiftedMirror->y(), 2); EXPECT_DOUBLE_EQ(shiftedMirror->z(), 3);
    auto sameIdCopy = std::make_shared<lc::entity::Point>(point, true);
    EXPECT_EQ(sameIdCopy->id(), point->id()); EXPECT_DOUBLE_EQ(sameIdCopy->z(), 3);
    auto locked = lc::meta::Layer::withState(layer, false, false, true);
    auto lock = std::make_shared<lc::operation::ReplaceLayer>(document, layer, locked);
    lock->execute();
    auto lockedPoint = std::dynamic_pointer_cast<const lc::entity::Point>(document->entityByID(point->id()));
    ASSERT_TRUE(lockedPoint); EXPECT_EQ(lockedPoint->layer(), locked); EXPECT_DOUBLE_EQ(lockedPoint->z(), 3);
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(moved); edit->execute();
    EXPECT_TRUE(edit->blockedByLockedLayer()); EXPECT_EQ(document->entityByID(point->id()), lockedPoint);
    edit->setAllowLockedLayerEdits(true); edit->execute();
    auto verify = [&] {
        auto result = std::dynamic_pointer_cast<const lc::entity::Point>(document->entityByID(point->id()));
        ASSERT_TRUE(result); EXPECT_EQ(result->layer(), locked); EXPECT_EQ(result->metaInfo(), metadata);
        EXPECT_DOUBLE_EQ(result->x(), 6); EXPECT_DOUBLE_EQ(result->y(), 8); EXPECT_DOUBLE_EQ(result->z(), 10);
    };
    verify(); edit->undo(); EXPECT_EQ(document->entityByID(point->id()), lockedPoint);
    edit->redo(); verify();
    auto target = std::make_shared<lc::meta::Layer>("POINT_TARGET"); document->addDocumentMetaType(target);
    auto change = std::make_shared<lc::operation::EntityBuilder>(document);
    change->setAllowLockedLayerEdits(true); change->appendEntity(moved->modify(target, metadata, nullptr)); change->execute();
    EXPECT_EQ(document->entityByID(point->id())->layer(), target);
    change->undo(); verify(); change->redo(); EXPECT_EQ(document->entityByID(point->id())->layer(), target);
}

TEST(BuilderTest, ImageForcedEditRebindsCurrentLayerAndPreservesGeometry) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = document->layerByName("0");
    auto metadata = lc::meta::MetaInfo::create();
    auto image = std::make_shared<lc::entity::Image>("image.png", lc::geo::Coordinate(1, 2, 3),
        lc::geo::Coordinate(1, 0, 1), lc::geo::Coordinate(0, 1, -1), 2, 3, 40, 60, 10, layer, metadata);
    document->insertEntity(image);
    auto moved = std::dynamic_pointer_cast<const lc::entity::Image>(image->move(lc::geo::Coordinate(5, 6, 7)));
    ASSERT_TRUE(moved);
    auto locked = lc::meta::Layer::withState(layer, false, false, true);
    auto lock = std::make_shared<lc::operation::ReplaceLayer>(document, layer, locked);
    lock->execute();
    auto lockedImage = std::dynamic_pointer_cast<const lc::entity::Image>(document->entityByID(image->id()));
    ASSERT_TRUE(lockedImage); ASSERT_EQ(lockedImage->layer(), locked);
    auto edit = std::make_shared<lc::operation::EntityBuilder>(document);
    edit->appendEntity(moved); edit->execute();
    EXPECT_TRUE(edit->blockedByLockedLayer()); EXPECT_EQ(document->entityByID(image->id()), lockedImage);
    edit->setAllowLockedLayerEdits(true); edit->execute();
    const auto verify = [&] {
        auto result = std::dynamic_pointer_cast<const lc::entity::Image>(document->entityByID(image->id()));
        ASSERT_TRUE(result); EXPECT_EQ(result->layer(), locked); EXPECT_EQ(result->metaInfo(), metadata);
        EXPECT_EQ(result->base(), lc::geo::Coordinate(6, 8, 10));
        EXPECT_EQ(result->uv(), image->uv()); EXPECT_EQ(result->vv(), image->vv());
    };
    verify(); edit->undo(); EXPECT_EQ(document->entityByID(image->id()), lockedImage);
    edit->redo(); verify();
    auto rotate = std::make_shared<lc::operation::EntityBuilder>(document);
    rotate->setAllowLockedLayerEdits(true);
    rotate->appendEntity(moved);
    rotate->appendOperation(std::make_shared<lc::operation::Push>());
    rotate->appendOperation(std::make_shared<lc::operation::Rotate>(lc::geo::Coordinate(), std::acos(-1.) / 2));
    rotate->execute();
    const auto verifyRotation = [&] {
        auto result = std::dynamic_pointer_cast<const lc::entity::Image>(document->entityByID(image->id()));
        ASSERT_TRUE(result); EXPECT_EQ(result->layer(), locked); EXPECT_EQ(result->metaInfo(), metadata);
        EXPECT_NEAR(result->base().x(), -8, 1e-12); EXPECT_NEAR(result->base().y(), 6, 1e-12);
        EXPECT_DOUBLE_EQ(result->base().z(), 10); EXPECT_DOUBLE_EQ(result->uv().z(), 1);
        EXPECT_DOUBLE_EQ(result->vv().z(), -1);
    };
    verifyRotation(); rotate->undo(); verify(); rotate->redo(); verifyRotation();
}

TEST(BuilderTest, PointQueriesAndDragRoundTripPreserve3dCoordinate) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = document->layerByName("0");
    auto point = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(1, 2, 3), layer);
    const auto nearest = point->nearestPointOnEntity(lc::geo::Coordinate(20, 30, 40));
    const auto path = point->nearestPointOnPath(lc::geo::Coordinate(20, 30, 40));
    EXPECT_DOUBLE_EQ(nearest.x(), 1); EXPECT_DOUBLE_EQ(nearest.y(), 2); EXPECT_DOUBLE_EQ(nearest.z(), 3);
    EXPECT_EQ(path, nearest); EXPECT_DOUBLE_EQ(path.z(), 3);
    const auto bounds = point->boundingBox();
    EXPECT_EQ(bounds.minP(), nearest); EXPECT_EQ(bounds.maxP(), nearest);
    EXPECT_DOUBLE_EQ(bounds.minP().z(), 3); EXPECT_DOUBLE_EQ(bounds.maxP().z(), 3);
    auto drag = point->dragPoints(); ASSERT_EQ(drag.size(), 1u); EXPECT_EQ(drag.at(0), nearest); EXPECT_DOUBLE_EQ(drag.at(0).z(), 3);
    auto unchanged = std::dynamic_pointer_cast<const lc::entity::Point>(point->setDragPoints(drag));
    ASSERT_TRUE(unchanged); EXPECT_EQ(unchanged->id(), point->id()); EXPECT_DOUBLE_EQ(unchanged->z(), 3);
    const auto snaps = point->snapPoints(lc::geo::Coordinate(1, 2, 3), lc::SimpleSnapConstrain(), 0.1, 1);
    ASSERT_EQ(snaps.size(), 1u); EXPECT_EQ(snaps.front().coordinate(), nearest); EXPECT_DOUBLE_EQ(snaps.front().coordinate().z(), 3);
    auto planar = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(1, 2), layer);
    const auto planarSnaps = planar->snapPoints(lc::geo::Coordinate(1, 2), lc::SimpleSnapConstrain(), 0.1, 1);
    ASSERT_EQ(planarSnaps.size(), 1u); EXPECT_EQ(planarSnaps.front().coordinate(), lc::geo::Coordinate(1, 2));
}

TEST(BuilderTest, ElevatedCircleAndArcBoundsKeepTheirPlane) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = document->layerByName("0");
    for (double z : {-70., 0., 70.}) {
        auto circle = std::make_shared<lc::entity::Circle>(lc::geo::Coordinate(2, 3, z), 5, layer);
        const auto circleBounds = circle->boundingBox();
        EXPECT_DOUBLE_EQ(circleBounds.minP().x(), -3);
        EXPECT_DOUBLE_EQ(circleBounds.minP().y(), -2);
        EXPECT_DOUBLE_EQ(circleBounds.maxP().x(), 7);
        EXPECT_DOUBLE_EQ(circleBounds.maxP().y(), 8);
        EXPECT_DOUBLE_EQ(circleBounds.minP().z(), z);
        EXPECT_DOUBLE_EQ(circleBounds.maxP().z(), z);
        for (int quadrant = 0; quadrant != 4; ++quadrant) {
            const double angle = quadrant * M_PI / 2.;
            auto arc = std::make_shared<lc::entity::Arc>(lc::geo::Coordinate(2, 3, z), 5,
                    angle - M_PI / 4., angle + M_PI / 4., true, layer);
            const auto bounds = arc->boundingBox();
            EXPECT_DOUBLE_EQ(bounds.minP().z(), z);
            EXPECT_DOUBLE_EQ(bounds.maxP().z(), z);
        }
    }
}

TEST(BuilderTest, AreaMergePreservesElevationRange) {
    const lc::geo::Area area(lc::geo::Coordinate(1, 2, 4), lc::geo::Coordinate(3, 4, 8));
    const auto withPoint = area.merge(lc::geo::Coordinate(5, 0, -2));
    EXPECT_DOUBLE_EQ(withPoint.minP().z(), -2);
    EXPECT_DOUBLE_EQ(withPoint.maxP().z(), 8);
    const auto withArea = area.merge(lc::geo::Area(lc::geo::Coordinate(-1, 0, -4),
                                                   lc::geo::Coordinate(2, 5, 12)));
    EXPECT_DOUBLE_EQ(withArea.minP().z(), -4);
    EXPECT_DOUBLE_EQ(withArea.maxP().z(), 12);
}

TEST(BuilderTest, AreaFromSizePreservesOriginElevation) {
    for (double z : {-4., 0., 4.}) {
        const lc::geo::Area area(lc::geo::Coordinate(1, 2, z), -3, 5);
        EXPECT_DOUBLE_EQ(area.minP().x(), -2);
        EXPECT_DOUBLE_EQ(area.maxP().x(), 1);
        EXPECT_DOUBLE_EQ(area.minP().y(), 2);
        EXPECT_DOUBLE_EQ(area.maxP().y(), 7);
        EXPECT_DOUBLE_EQ(area.minP().z(), z);
        EXPECT_DOUBLE_EQ(area.maxP().z(), z);
    }
}

TEST(BuilderTest, SplinePointHullTracksElevationOnMove) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = document->layerByName("0");
    const std::vector<lc::geo::Coordinate> points{{0, 1, 10}, {2, 3, -4}, {4, 1, 7}};
    const std::vector<lc::geo::Coordinate> empty;
    for (bool fitOnly : {false, true}) {
        auto spline = std::make_shared<lc::entity::Spline>(fitOnly ? empty : points,
                fitOnly ? std::vector<double>{} : std::vector<double>{0, 0, 0, 1, 1, 1},
                fitOnly ? points : empty, 2, false, 0.,
                0., 0., 0., 0., 0., 0., 0., 0., 1.,
                static_cast<lc::geo::Spline::splineflag>(0), layer);
        EXPECT_DOUBLE_EQ(spline->boundingBox().minP().z(), -4);
        EXPECT_DOUBLE_EQ(spline->boundingBox().maxP().z(), 10);
        const auto moved = spline->move(lc::geo::Coordinate(0, 0, 5));
        EXPECT_DOUBLE_EQ(moved->boundingBox().minP().z(), 1);
        EXPECT_DOUBLE_EQ(moved->boundingBox().maxP().z(), 15);
    }
}

TEST(BuilderTest, ElevatedEllipseBoundsKeepTheirPlane) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto layer = document->layerByName("0");
    for (double z : {-70., 0., 70.}) {
        for (double angle : {0., M_PI / 4.}) {
            auto major = lc::geo::Coordinate(5, 0).rotate(angle);
            for (bool partial : {false, true}) {
                auto ellipse = std::make_shared<lc::entity::Ellipse>(lc::geo::Coordinate(2, 3, z),
                        major, 2., partial ? 0.2 : 0., partial ? 1.7 : 2. * M_PI, false, layer);
                const auto bounds = ellipse->boundingBox();
                EXPECT_DOUBLE_EQ(bounds.minP().z(), z);
                EXPECT_DOUBLE_EQ(bounds.maxP().z(), z);
                for (const auto &point : ellipse->findBoxPoints()) {
                    EXPECT_DOUBLE_EQ(point.z(), z);
                    EXPECT_GE(point.x(), bounds.minP().x() - 1e-12);
                    EXPECT_LE(point.x(), bounds.maxP().x() + 1e-12);
                    EXPECT_GE(point.y(), bounds.minP().y() - 1e-12);
                    EXPECT_LE(point.y(), bounds.maxP().y() + 1e-12);
                }
            }
        }
    }
}
