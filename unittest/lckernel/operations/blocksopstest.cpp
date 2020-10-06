#include <gtest/gtest.h>

#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/operations/blockops.h>

using namespace lc;
using namespace storage;
using namespace meta;

TEST(BlockOps, AddBlock) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());
    auto block = std::make_shared<lc::meta::Block>("Name", geo::Coordinate());
    auto op = std::make_shared<operation::AddBlock>(document, block);

    //Remove existing blocks
    for(auto& block : document->blocks()) {
        document->removeDocumentMetaType(block);
    }

    auto blocks = document->blocks();
    EXPECT_EQ(0, blocks.size());

    op->execute();
    blocks = document->blocks();
    EXPECT_EQ(1, blocks.size());
    EXPECT_EQ(block, *blocks.begin());

    op->undo();
    blocks = document->blocks();
    EXPECT_EQ(0, blocks.size());

    op->redo();
    blocks = document->blocks();
    EXPECT_EQ(1, blocks.size());
    EXPECT_EQ(block, *blocks.begin());
}

TEST(BlockOps, RemoveBlock) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());

    //Remove existing blocks
    for(auto& block : document->blocks()) {
        document->removeDocumentMetaType(block);
    }


    auto block = std::make_shared<lc::meta::Block>("Name", geo::Coordinate());

    auto addOp = std::make_shared<operation::AddBlock>(document, block);
    auto remOp = std::make_shared<operation::RemoveBlock>(document, block);

    addOp->execute();
    auto blocks = document->blocks();

    remOp->execute();
    blocks = document->blocks();
    EXPECT_EQ(0, blocks.size());

    remOp->undo();
    blocks = document->blocks();
    EXPECT_EQ(1, blocks.size());
    EXPECT_EQ(block, *blocks.begin());

    remOp->redo();
    blocks = document->blocks();
    EXPECT_EQ(0, blocks.size());
}

TEST(BlockOps, ReplaceBlock) {
    auto document = std::make_shared<DocumentImpl>(std::make_shared<StorageManagerImpl>());

    //Remove existing blocks
    for(auto& block : document->blocks()) {
        document->removeDocumentMetaType(block);
    }


    auto block = std::make_shared<lc::meta::Block>("Name", geo::Coordinate());
    auto block2 = std::make_shared<lc::meta::Block>("Name 2", geo::Coordinate());

    auto addOp = std::make_shared<operation::AddBlock>(document, block);
    auto repOp = std::make_shared<operation::ReplaceBlock>(document, block, block2);

    addOp->execute();
    auto blocks = document->blocks();

    repOp->execute();
    blocks = document->blocks();
    EXPECT_EQ(1, blocks.size());
    EXPECT_EQ(block2, *blocks.begin());

    repOp->undo();
    blocks = document->blocks();
    EXPECT_EQ(1, blocks.size());
    EXPECT_EQ(block, *blocks.begin());

    repOp->redo();
    blocks = document->blocks();
    EXPECT_EQ(1, blocks.size());
    EXPECT_EQ(block2, *blocks.begin());
}