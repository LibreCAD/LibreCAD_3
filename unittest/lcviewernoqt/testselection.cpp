#include <gtest/gtest.h>
#include "documentcanvas.h"
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>

#include <cad/operations/entitybuilder.h>
#include <cad/operations/documentoperation.h>
#include <cad/meta/layer.h>
#include <cad/operations/layerops.h>
#include <cad/primitive/line.h>
#include "drawitems/lcvdrawitem.h"

TEST(SelectionTest, NormalSelection) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 5, 10, true);
    docCanvas->closeSelection();

    EXPECT_EQ(1, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, IntersectionSelection) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 5, 5, false);
    docCanvas->closeSelection();

    EXPECT_EQ(1, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, AddToSelection) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 1, 1, false);
    docCanvas->closeSelection();

    docCanvas->makeSelection(9, 0, 1, 1, false);
    docCanvas->closeSelection();

    EXPECT_EQ(2, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, Reselect) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 10, 1, false);
    docCanvas->closeSelection();

    docCanvas->makeSelection(9, 0, 1, 1, false);
    docCanvas->closeSelection();

    EXPECT_EQ(1, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, ClearSelection) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 1, 1, false);
    docCanvas->closeSelection();

    docCanvas->makeSelection(9, 0, 1, 1, false);
    docCanvas->closeSelection();

    EXPECT_EQ(2, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, Deselect) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 1, 1, false);
    docCanvas->makeSelection(-1, -1, 0, 0, false);
    docCanvas->closeSelection();

    EXPECT_EQ(0, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, DeselectAddTo) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 1, 1, false);
    docCanvas->closeSelection();

    docCanvas->makeSelection(9, 0, 1, 1, false);
    docCanvas->makeSelection(11, 0, 0, 0, false);
    docCanvas->closeSelection();

    EXPECT_EQ(1, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}
