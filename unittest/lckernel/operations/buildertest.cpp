#include <gtest/gtest.h>
#include <cad/dochelpers/documentimpl.h>
#include <cad/dochelpers/storagemanagerimpl.h>
#include <cad/operations/builder.h>
#include <cad/primitive/line.h>

TEST(BuilderTest, Append) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

	auto builder = std::make_shared<lc::operation::Builder>(document);
	auto entity = std::make_shared<lc::entity::Line>(
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(100, 100),
			std::make_shared<const lc::Layer>()
	);

	builder->append(entity);
	builder->execute();

	EXPECT_EQ(1, document->entityContainer().asVector().size());

	EXPECT_EQ(entity, *document->entityContainer().asVector().begin()) << "Wrong entity was appended to Document";
}

TEST(BuilderTest, Remove) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

	auto builder = std::make_shared<lc::operation::Builder>(document);
	auto entity = std::make_shared<lc::entity::Line>(
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(100, 100),
			std::make_shared<const lc::Layer>()
	);

	builder->append(entity);
	builder->execute();

	builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(entity);
	builder->push();
	builder->remove();
	builder->execute();

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Entity was not deleted";
}

TEST(BuilderTest, UndoRedo) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	std::shared_ptr<lc::operation::Builder> builder;

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

	auto line1 = std::make_shared<lc::entity::Line>(
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(100, 100),
			std::make_shared<const lc::Layer>()
	);
	auto line2 = line1->move(lc::geo::Coordinate());

	//Undo/Redo append
	builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(line1);
	builder->execute();

	builder->undo();

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Entity still present after undo append";

	builder->redo();
	EXPECT_EQ(1, document->entityContainer().asVector().size()) << "Entity not present after redo append";
	EXPECT_EQ(line1, *document->entityContainer().asVector().begin()) << "Entity not present after redo append";

	//Undo/Redo remove
	builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(line1);
	builder->execute();

	builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(line1);
	builder->push();
	builder->remove();
	builder->execute();

	builder->undo();

	EXPECT_EQ(1, document->entityContainer().asVector().size()) << "Builder didn't undo remove";
	EXPECT_EQ(line1, *document->entityContainer().asVector().begin()) << "Builder didn't undo remove";

	builder->redo();

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Builder didn't redo remove";

	//Undo/Redo update
	builder = std::make_shared<lc::operation::Builder>(document);

	builder->append(line1);
	builder->execute();

	builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(line2);
	builder->execute();

	builder->undo();

	EXPECT_EQ(line1, *document->entityContainer().asVector().begin()) << "Builder didn't undo update";

	builder->redo();

	EXPECT_EQ(line2, *document->entityContainer().asVector().begin()) << "Builder didn't redo update";
}

TEST(BuilderTest, Move) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	std::shared_ptr<lc::operation::Builder> builder;

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

	auto entity = std::make_shared<lc::entity::Line>(
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(100, 100),
			std::make_shared<const lc::Layer>()
	);
	auto offset = lc::geo::Coordinate(100, 100);

	builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(entity);
	builder->push();
	builder->move(offset);

	builder->execute();

	auto manualOperation = std::static_pointer_cast<const lc::entity::Line>(entity->move(offset));
	auto builderOperation = std::static_pointer_cast<const lc::entity::Line>(*document->entityContainer().asVector().begin());

	EXPECT_EQ(manualOperation->start(), builderOperation->start());
	EXPECT_EQ(manualOperation->end(), builderOperation->end());
}

TEST(BuilderTest, Copy) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	std::shared_ptr<lc::operation::Builder> builder;

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

	auto entity = std::make_shared<lc::entity::Line>(
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(100, 100),
			std::make_shared<const lc::Layer>()
	);
	auto offset = lc::geo::Coordinate(100, 100);

	builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(entity);
	builder->push();
	builder->copy(offset);

	builder->execute();

	EXPECT_EQ(2, document->entityContainer().asVector().size());

	auto manualOperation = std::static_pointer_cast<const lc::entity::Line>(entity->copy(offset));
	auto builderOperation = std::static_pointer_cast<const lc::entity::Line>(*(document->entityContainer().asVector().begin() + 1));

	EXPECT_EQ(manualOperation->start(), builderOperation->start());
	EXPECT_EQ(manualOperation->end(), builderOperation->end());
}

TEST(BuilderTest, Rotate) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	std::shared_ptr<lc::operation::Builder> builder;

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

	auto entity = std::make_shared<lc::entity::Line>(
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(100, 100),
			std::make_shared<const lc::Layer>()
	);
	auto center = entity->start();
	const double angle = 1;

	builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(entity);
	builder->push();
	builder->rotate(center, angle);

	builder->execute();

	auto manualOperation = std::static_pointer_cast<const lc::entity::Line>(entity->rotate(center, angle));
	auto builderOperation = std::static_pointer_cast<const lc::entity::Line>(*document->entityContainer().asVector().begin());

	EXPECT_EQ(manualOperation->start(), builderOperation->start());
	EXPECT_EQ(manualOperation->end(), builderOperation->end());
}

TEST(BuilderTest, Scale) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	std::shared_ptr<lc::operation::Builder> builder;

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

	auto entity = std::make_shared<lc::entity::Line>(
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(100, 100),
			std::make_shared<const lc::Layer>()
	);
	auto center = entity->start();
	auto factor = lc::geo::Coordinate(2, 2);

	builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(entity);
	builder->push();
	builder->scale(center, factor);

	builder->execute();

	auto manualOperation = std::static_pointer_cast<const lc::entity::Line>(entity->scale(center, factor));
	auto builderOperation = std::static_pointer_cast<const lc::entity::Line>(*document->entityContainer().asVector().begin());

	EXPECT_EQ(manualOperation->start(), builderOperation->start());
	EXPECT_EQ(manualOperation->end(), builderOperation->end());
}

TEST(BuilderTest, Repeat) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	std::shared_ptr<lc::operation::Builder> builder;

	EXPECT_EQ(0, document->entityContainer().asVector().size()) << "Document is not empty after creation";

	auto entity = std::make_shared<lc::entity::Line>(
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(100, 100),
			std::make_shared<const lc::Layer>()
	);
	auto offset = lc::geo::Coordinate(100, 100);

	builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(entity);
	builder->push();
	builder->move(offset);
	builder->repeat(3);

	builder->execute();

	auto movedEntity = entity->move(offset)->move(offset)->move(offset);
	auto manualOperation = std::static_pointer_cast<const lc::entity::Line>(movedEntity);
	auto builderOperation = std::static_pointer_cast<const lc::entity::Line>(*document->entityContainer().asVector().begin());

	EXPECT_EQ(manualOperation->start(), builderOperation->start());
	EXPECT_EQ(manualOperation->end(), builderOperation->end());
}

TEST(BuilderTest, SelectByLayer) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	auto builder = std::make_shared<lc::operation::Builder>(document);

	auto layer = std::make_shared<const lc::Layer>("NewLayer", lc::Color(0, 0, 0));

	auto entityInsideLayer = std::make_shared<lc::entity::Line>(
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(100, 100),
			layer
	);

	auto entityOutsideLayer = std::make_shared<lc::entity::Line>(
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(100, 100),
			std::make_shared<const lc::Layer>()
	);

	auto offset = lc::geo::Coordinate(100, 100);

	builder->append(entityInsideLayer);
	builder->append(entityOutsideLayer);

	builder->selectByLayer(layer);
	builder->move(offset);
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