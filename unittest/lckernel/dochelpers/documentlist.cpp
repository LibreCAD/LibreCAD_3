#include <gtest/gtest.h>
#include <memory>
#include <cad/dochelpers/documentimpl.h>
#include <cad/dochelpers/documentlist.h>
#include <cad/dochelpers/storagemanagerimpl.h>
#include <cad/meta/customentitystorage.h>
#include <cad/operations/blockops.h>
#include <cad/builders/insert.h>
#include <cad/operations/entitybuilder.h>
#include <cad/primitive/insert.h>

bool eventReceived = false;

void onEventReceived(const lc::NewWaitingCustomEntityEvent& event) {
    eventReceived = true;
}

TEST(DocumentList, NewWaitingCustomEntity) {
    lc::DocumentList::getInstance().newWaitingCustomEntityEvent().connect<onEventReceived>();

    auto doc1 = std::make_shared<lc::DocumentImpl>(std::make_shared<lc::StorageManagerImpl>());

    auto customEntityStorage = std::make_shared<lc::CustomEntityStorage>("plugin", "entity", lc::geo::Coordinate());
    std::make_shared<lc::operation::AddBlock>(doc1, customEntityStorage)->execute();

    auto builder = lc::builder::InsertBuilder();
    builder.setDisplayBlock(customEntityStorage);
    builder.setDocument(doc1);
    builder.setLayer(std::make_shared<lc::Layer>());

    std::make_shared<lc::operation::EntityBuilder>(doc1)->appendEntity(builder.build())->execute();

    ASSERT_TRUE(eventReceived);
    ASSERT_EQ(1, lc::DocumentList::getInstance().waitingCustomEntities("plugin").size());
}