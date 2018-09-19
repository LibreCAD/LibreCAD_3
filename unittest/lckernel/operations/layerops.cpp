#include <gtest/gtest.h>
#include <cad/dochelpers/storagemanagerimpl.h>
#include <cad/dochelpers/documentimpl.h>
#include <cad/operations/layerops.h>
#include <cad/operations/entitybuilder.h>

TEST(LayerOps, ReplaceLayer) {
    auto document = std::make_shared<lc::DocumentImpl>(std::make_shared<lc::StorageManagerImpl>());

    auto layer = std::make_shared<lc::Layer>("1");
    auto layer2 = std::make_shared<lc::Layer>("2");
    auto addLayer = std::make_shared<lc::operation::AddLayer>(document, layer);
    addLayer->execute();

    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), layer);
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