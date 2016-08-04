#include <gtest/gtest.h>
#include "documentcanvas.h"
#include <cad/dochelpers/documentimpl.h>
#include <cad/dochelpers/storagemanagerimpl.h>

#include <cad/operations/builder.h>
#include <cad/operations/documentoperation.h>
#include <cad/meta/layer.h>
#include <cad/operations/layerops.h>
#include <cad/primitive/line.h>
#include "drawitems/lcvdrawitem.h"

TEST(SelectionTest, NormalSelection) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	auto docCanvas = std::make_shared<LCViewer::DocumentCanvas>(document);

	auto layer = std::make_shared<lc::Layer>("0", lc::Color(1., 1., 1., 1.));
	std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
	al->execute();

	auto builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
	builder->execute();

	docCanvas->makeSelection(0, 0, 5, 10, true, false);
	docCanvas->closeSelection();

	EXPECT_EQ(1, docCanvas->selection().asVector().size());

	unsigned int i = 0;

	docCanvas->entityContainer().each<LCViewer::LCVDrawItem>([&](LCViewer::LCVDrawItem_SPtr di) {
		if(di->selected() == true) {
			i++;
		}
	});

	EXPECT_TRUE(i == docCanvas->selection().asVector().size());
}

TEST(SelectionTest, IntersectionSelection) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	auto docCanvas = std::make_shared<LCViewer::DocumentCanvas>(document);

	auto layer = std::make_shared<lc::Layer>("0", lc::Color(1., 1., 1., 1.));
	std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
	al->execute();

	auto builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
	builder->execute();

	docCanvas->makeSelection(0, 0, 5, 5, false, false);
	docCanvas->closeSelection();

	EXPECT_EQ(1, docCanvas->selection().asVector().size());

	unsigned int i = 0;

	docCanvas->entityContainer().each<LCViewer::LCVDrawItem>([&](LCViewer::LCVDrawItem_SPtr di) {
		if(di->selected() == true) {
			i++;
		}
	});

	EXPECT_TRUE(i == docCanvas->selection().asVector().size());
}

TEST(SelectionTest, AddToSelection) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	auto docCanvas = std::make_shared<LCViewer::DocumentCanvas>(document);

	auto layer = std::make_shared<lc::Layer>("0", lc::Color(1., 1., 1., 1.));
	std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
	al->execute();

	auto builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
	builder->execute();

	docCanvas->makeSelection(0, 0, 1, 1, false, false);
	docCanvas->closeSelection();

	docCanvas->makeSelection(9, 0, 1, 1, false, true);
	docCanvas->closeSelection();

	EXPECT_EQ(2, docCanvas->selection().asVector().size());

	unsigned int i = 0;

	docCanvas->entityContainer().each<LCViewer::LCVDrawItem>([&](LCViewer::LCVDrawItem_SPtr di) {
		if(di->selected() == true) {
			i++;
		}
	});

	EXPECT_TRUE(i == docCanvas->selection().asVector().size());
}

TEST(SelectionTest, Reselect) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	auto docCanvas = std::make_shared<LCViewer::DocumentCanvas>(document);

	auto layer = std::make_shared<lc::Layer>("0", lc::Color(1., 1., 1., 1.));
	std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
	al->execute();

	auto builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
	builder->execute();

	docCanvas->makeSelection(0, 0, 10, 1, false, false);
	docCanvas->closeSelection();

	docCanvas->makeSelection(9, 0, 1, 1, false, true);
	docCanvas->closeSelection();

	EXPECT_EQ(1, docCanvas->selection().asVector().size());

	unsigned int i = 0;

	docCanvas->entityContainer().each<LCViewer::LCVDrawItem>([&](LCViewer::LCVDrawItem_SPtr di) {
		if(di->selected() == true) {
			i++;
		}
	});

	EXPECT_TRUE(i == docCanvas->selection().asVector().size());
}

TEST(SelectionTest, ClearSelection) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	auto docCanvas = std::make_shared<LCViewer::DocumentCanvas>(document);

	auto layer = std::make_shared<lc::Layer>("0", lc::Color(1., 1., 1., 1.));
	std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
	al->execute();

	auto builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
	builder->execute();

	docCanvas->makeSelection(0, 0, 1, 1, false, false);
	docCanvas->closeSelection();

	docCanvas->makeSelection(9, 0, 1, 1, false, false);
	docCanvas->closeSelection();

	EXPECT_EQ(1, docCanvas->selection().asVector().size());

	unsigned int i = 0;

	docCanvas->entityContainer().each<LCViewer::LCVDrawItem>([&](LCViewer::LCVDrawItem_SPtr di) {
		if(di->selected() == true) {
			i++;
		}
	});

	EXPECT_TRUE(i == docCanvas->selection().asVector().size());
}

TEST(SelectionTest, Deselect) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	auto docCanvas = std::make_shared<LCViewer::DocumentCanvas>(document);

	auto layer = std::make_shared<lc::Layer>("0", lc::Color(1., 1., 1., 1.));
	std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
	al->execute();

	auto builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
	builder->execute();

	docCanvas->makeSelection(0, 0, 1, 1, false, false);
	docCanvas->makeSelection(-1, -1, 0, 0, false, false);
	docCanvas->closeSelection();

	EXPECT_EQ(0, docCanvas->selection().asVector().size());

	unsigned int i = 0;

	docCanvas->entityContainer().each<LCViewer::LCVDrawItem>([&](LCViewer::LCVDrawItem_SPtr di) {
		if(di->selected() == true) {
			i++;
		}
	});

	EXPECT_TRUE(i == docCanvas->selection().asVector().size());
}

TEST(SelectionTest, DeselectAddTo) {
	auto storageManager = std::make_shared<lc::StorageManagerImpl>();
	auto document = std::make_shared<lc::DocumentImpl>(storageManager);
	auto docCanvas = std::make_shared<LCViewer::DocumentCanvas>(document);

	auto layer = std::make_shared<lc::Layer>("0", lc::Color(1., 1., 1., 1.));
	std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
	al->execute();

	auto builder = std::make_shared<lc::operation::Builder>(document);
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
	builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
	builder->execute();

	docCanvas->makeSelection(0, 0, 1, 1, false, false);
	docCanvas->closeSelection();

	docCanvas->makeSelection(9, 0, 1, 1, false, true);
	docCanvas->makeSelection(11, 0, 0, 0, false, true);
	docCanvas->closeSelection();

	EXPECT_EQ(1, docCanvas->selection().asVector().size());

	unsigned int i = 0;

	docCanvas->entityContainer().each<LCViewer::LCVDrawItem>([&](LCViewer::LCVDrawItem_SPtr di) {
		if(di->selected() == true) {
			i++;
		}
	});

	EXPECT_TRUE(i == docCanvas->selection().asVector().size());
}