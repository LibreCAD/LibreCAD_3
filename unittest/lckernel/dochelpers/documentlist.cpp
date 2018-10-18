#include <gtest/gtest.h>
#include <memory>
#include <cad/storage/documentimpl.h>
#include <cad/storage/documentlist.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/meta/customentitystorage.h>
#include <cad/operations/blockops.h>
#include <cad/builders/insert.h>
#include <cad/operations/entitybuilder.h>
#include <cad/primitive/insert.h>

bool eventReceived = false;

void onEventReceived(const lc::event::NewWaitingCustomEntityEvent& event) {
    eventReceived = true;
}

TEST(DocumentList, NewWaitingCustomEntity) {
    lc::storage::DocumentList::getInstance().newWaitingCustomEntityEvent().connect<onEventReceived>();

    auto doc1 = std::make_shared<lc::storage::DocumentImpl>(std::make_shared<lc::storage::StorageManagerImpl>());

    auto customEntityStorage = std::make_shared<lc::meta::CustomEntityStorage>("plugin", "entity", lc::geo::Coordinate());
    std::make_shared<lc::operation::AddBlock>(doc1, customEntityStorage)->execute();

    auto builder = lc::builder::InsertBuilder();
    builder.setDisplayBlock(customEntityStorage);
    builder.setDocument(doc1);
    builder.setLayer(std::make_shared<lc::meta::Layer>());

    std::make_shared<lc::operation::EntityBuilder>(doc1)->appendEntity(builder.build())->execute();

    ASSERT_TRUE(eventReceived);
    ASSERT_EQ(1, lc::storage::DocumentList::getInstance().waitingCustomEntities("plugin").size());
}