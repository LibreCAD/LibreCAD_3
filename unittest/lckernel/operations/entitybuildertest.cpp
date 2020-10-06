#include <gtest/gtest.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/operations/entitybuilder.h>
#include <cad/primitive/line.h>

TEST(EntityBuilderTest, Append) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    auto entity = std::make_shared<lc::entity::Line>(
                      lc::geo::Coordinate(0, 0),
                      lc::geo::Coordinate(100, 100),
                      std::make_shared<const lc::meta::Layer>(),
                      nullptr
                  );

    builder->appendEntity(entity);
    builder->execute();

    EXPECT_EQ(1, document->entityContainer().asVector().size());

    EXPECT_EQ(entity, *document->entityContainer().asVector().begin()) << "Wrong entity was appended to Document";
}

TEST(EntityBuilderTest, Remove) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    auto entity = std::make_shared<lc::entity::Line>(
                      lc::geo::Coordinate(0, 0),
                      lc::geo::Coordinate(100, 100),
                      std::make_shared<const lc::meta::Layer>(),
                      nullptr
                  );

    builder->appendEntity(entity);
    builder->execute();

    builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(entity);
    builder->appendOperation(std::make_shared<lc::operation::Push>());
    builder->appendOperation(std::make_shared<lc::operation::Remove>());
    builder->execute();

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Entity was not deleted";
}

TEST(EntityBuilderTest, UndoRedo) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    std::shared_ptr<lc::operation::EntityBuilder> builder;

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

    auto line1 = std::make_shared<lc::entity::Line>(
                     lc::geo::Coordinate(0, 0),
                     lc::geo::Coordinate(100, 100),
                     std::make_shared<const lc::meta::Layer>(),
                     nullptr
                 );
    auto line2 = line1->move(lc::geo::Coordinate());

    //Undo/Redo append
    builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(line1);
    builder->execute();

    builder->undo();

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Entity still present after undo append";

    builder->redo();
    EXPECT_EQ(1, document->entityContainer().asVector().size()) << "Entity not present after redo append";
    EXPECT_EQ(line1, *document->entityContainer().asVector().begin()) << "Entity not present after redo append";

    //Undo/Redo remove
    builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(line1);
    builder->execute();

    builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(line1);
    builder->appendOperation(std::make_shared<lc::operation::Push>());
    builder->appendOperation(std::make_shared<lc::operation::Remove>());
    builder->execute();

    builder->undo();

    EXPECT_EQ(1, document->entityContainer().asVector().size()) << "EntityBuilder didn't undo remove";
    EXPECT_EQ(line1, *document->entityContainer().asVector().begin()) << "EntityBuilder didn't undo remove";

    builder->redo();

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "EntityBuilder didn't redo remove";

    //Undo/Redo update
    builder = std::make_shared<lc::operation::EntityBuilder>(document);

    builder->appendEntity(line1);
    builder->execute();

    builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(line2);
    builder->execute();

    builder->undo();

    EXPECT_EQ(line1, *document->entityContainer().asVector().begin()) << "EntityBuilder didn't undo update";

    builder->redo();

    EXPECT_EQ(line2, *document->entityContainer().asVector().begin()) << "EntityBuilder didn't redo update";
}

TEST(EntityBuilderTest, Move) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    std::shared_ptr<lc::operation::EntityBuilder> builder;

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

    auto entity = std::make_shared<lc::entity::Line>(
                      lc::geo::Coordinate(0, 0),
                      lc::geo::Coordinate(100, 100),
                      std::make_shared<const lc::meta::Layer>(),
                      nullptr
                  );
    auto offset = lc::geo::Coordinate(100, 100);

    builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(entity);
    builder->appendOperation(std::make_shared<lc::operation::Push>());
    builder->appendOperation(std::make_shared<lc::operation::Move>(offset));

    builder->execute();

    auto manualOperation = std::static_pointer_cast<const lc::entity::Line>(entity->move(offset));
    auto builderOperation = std::static_pointer_cast<const lc::entity::Line>(*document->entityContainer().asVector().begin());

    EXPECT_EQ(manualOperation->start(), builderOperation->start());
    EXPECT_EQ(manualOperation->end(), builderOperation->end());
}

TEST(EntityBuilderTest, Copy) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    std::shared_ptr<lc::operation::EntityBuilder> builder;

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

    auto entity = std::make_shared<lc::entity::Line>(
                      lc::geo::Coordinate(0, 0),
                      lc::geo::Coordinate(100, 100),
                      std::make_shared<const lc::meta::Layer>(),
                      nullptr
                  );
    auto offset = lc::geo::Coordinate(100, 100);

    builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(entity);
    builder->appendOperation(std::make_shared<lc::operation::Push>());
    builder->appendOperation(std::make_shared<lc::operation::Copy>(offset));

    builder->execute();

    EXPECT_EQ(2, document->entityContainer().asVector().size());

    auto manualOperation = std::static_pointer_cast<const lc::entity::Line>(entity->copy(offset));
    auto builderOperation = std::static_pointer_cast<const lc::entity::Line>(*(document->entityContainer().asVector().begin() + 1));

    EXPECT_EQ(manualOperation->start(), builderOperation->start());
    EXPECT_EQ(manualOperation->end(), builderOperation->end());
}

TEST(EntityBuilderTest, Rotate) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    std::shared_ptr<lc::operation::EntityBuilder> builder;

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

    auto entity = std::make_shared<lc::entity::Line>(
                      lc::geo::Coordinate(0, 0),
                      lc::geo::Coordinate(100, 100),
                      std::make_shared<const lc::meta::Layer>(),
                      nullptr
                  );
    auto center = entity->start();
    const double angle = 1;

    builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(entity);
    builder->appendOperation(std::make_shared<lc::operation::Push>());
    builder->appendOperation(std::make_shared<lc::operation::Rotate>(center, angle));

    builder->execute();

    auto manualOperation = std::static_pointer_cast<const lc::entity::Line>(entity->rotate(center, angle));
    auto builderOperation = std::static_pointer_cast<const lc::entity::Line>(*document->entityContainer().asVector().begin());

    EXPECT_EQ(manualOperation->start(), builderOperation->start());
    EXPECT_EQ(manualOperation->end(), builderOperation->end());
}

TEST(EntityBuilderTest, Scale) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    std::shared_ptr<lc::operation::EntityBuilder> builder;

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

    auto entity = std::make_shared<lc::entity::Line>(
                      lc::geo::Coordinate(0, 0),
                      lc::geo::Coordinate(100, 100),
                      std::make_shared<const lc::meta::Layer>(),
                      nullptr
                  );
    auto center = entity->start();
    auto factor = lc::geo::Coordinate(2, 2);

    builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(entity);
    builder->appendOperation(std::make_shared<lc::operation::Push>());
    builder->appendOperation(std::make_shared<lc::operation::Scale>(center, factor));

    builder->execute();

    auto manualOperation = std::static_pointer_cast<const lc::entity::Line>(entity->scale(center, factor));
    auto builderOperation = std::static_pointer_cast<const lc::entity::Line>(*document->entityContainer().asVector().begin());

    EXPECT_EQ(manualOperation->start(), builderOperation->start());
    EXPECT_EQ(manualOperation->end(), builderOperation->end());
}

TEST(EntityBuilderTest, Repeat) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    std::shared_ptr<lc::operation::EntityBuilder> builder;

    EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

    auto entity = std::make_shared<lc::entity::Line>(
                      lc::geo::Coordinate(0, 0),
                      lc::geo::Coordinate(100, 100),
                      std::make_shared<const lc::meta::Layer>(),
                      nullptr
                  );
    auto offset = lc::geo::Coordinate(100, 100);

    builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(entity);
    builder->appendOperation(std::make_shared<lc::operation::Push>());
    builder->appendOperation(std::make_shared<lc::operation::Move>(offset));
    builder->appendOperation(std::make_shared<lc::operation::Loop>(3));

    builder->execute();

    auto movedEntity = entity->move(offset)->move(offset)->move(offset);
    auto manualOperation = std::static_pointer_cast<const lc::entity::Line>(movedEntity);
    auto builderOperation = std::static_pointer_cast<const lc::entity::Line>(*document->entityContainer().asVector().begin());

    EXPECT_EQ(manualOperation->start(), builderOperation->start());
    EXPECT_EQ(manualOperation->end(), builderOperation->end());
}

TEST(EntityBuilderTest, SelectByLayer) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);

    auto layer = std::make_shared<const lc::meta::Layer>();

    auto entityInsideLayer = std::make_shared<lc::entity::Line>(
                                 lc::geo::Coordinate(0, 0),
                                 lc::geo::Coordinate(100, 100),
                                 layer,
                                 nullptr
                             );

    auto entityOutsideLayer = std::make_shared<lc::entity::Line>(
                                  lc::geo::Coordinate(0, 0),
                                  lc::geo::Coordinate(100, 100),
                                  std::make_shared<const lc::meta::Layer>(),
                                  nullptr
                              );

    auto offset = lc::geo::Coordinate(100, 100);

    builder->appendEntity(entityInsideLayer);
    builder->appendEntity(entityOutsideLayer);

    builder->appendOperation(std::make_shared<lc::operation::SelectByLayer>(layer));
    builder->appendOperation(std::make_shared<lc::operation::Move>(offset));
    builder->execute();


    auto firstEntity = std::static_pointer_cast<const lc::entity::Line>(*document->entityContainer().asVector().begin());
    auto secondEntity = std::static_pointer_cast<const lc::entity::Line>(*(document->entityContainer().asVector().begin() + 1));

    auto expectedEntity1 = std::static_pointer_cast<const lc::entity::Line>(entityInsideLayer->move(offset));
    auto expectedEntity2 = entityOutsideLayer;

    bool firstEntity_isExpected1 =
        firstEntity->start() == expectedEntity1->start() &&
        firstEntity->end() == expectedEntity1->end();

    bool firstEntity_isExpected2 =
        firstEntity->start() == expectedEntity2->start() &&
        firstEntity->end() == expectedEntity2->end();

    bool secondEntity_isExpected1 =
        secondEntity->start() == expectedEntity1->start() &&
        secondEntity->end() == expectedEntity1->end();

    bool secondEntity_isExpected2 =
        secondEntity->start() == expectedEntity2->start() &&
        secondEntity->end() == expectedEntity2->end();

    EXPECT_TRUE((firstEntity_isExpected1 && secondEntity_isExpected2) ||
                (firstEntity_isExpected2 && secondEntity_isExpected1));
}