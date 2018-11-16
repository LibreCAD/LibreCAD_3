#include <gtest/gtest.h>

#include <QApplication>

#include <luainterface.h>

#include "uitests.h"

TEST(LuaUIOperationsTest, Move) {
	QApplication app(argc, argv);
	LuaInterface luaInterface;
	auto L = luaInterface.luaState();

	luaInterface.hideUI(true);
	luaInterface.initLua();

	auto mdiArea = LuaIntf::Lua::getGlobal<QMdiArea*>(L, "mdiArea");

	if(mdiArea->subWindowList().count() == 0) {
		LuaIntf::LuaRef(L, "new_file")();
	}

	mdiArea->setActiveSubWindow(mdiArea->subWindowList().at(0));

	auto mdiChild = dynamic_cast<CadMdiChild*>(mdiArea->activeSubWindow()->widget());
	auto storageManager = mdiChild->storageManager();
	auto documentCanvas = mdiChild->viewer()->documentCanvas();

	lc::entity::Line_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();

	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());



	LuaIntf::LuaRef(L, "create_line")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "Line was not created";



	documentCanvas->makeSelection(0, 0, 100, 100, false, false);
	documentCanvas->closeSelection();

	EXPECT_EQ(1, mdiChild->selection().size()) << "Entity not selected";



	LuaIntf::LuaRef(L, "move_selected_entities")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));

	entities = storageManager->entityContainer().asVector();
	createdEntity = std::dynamic_pointer_cast<const lc::entity::Line>(entities.at(0));

	EXPECT_EQ(lc::geo::Coordinate(100, 100), createdEntity->start());
	EXPECT_EQ(lc::geo::Coordinate(200, 200), createdEntity->end());
}

TEST(LuaUIOperationsTest, Rotate) {
	QApplication app(argc, argv);
	LuaInterface luaInterface;
	auto L = luaInterface.luaState();

	luaInterface.hideUI(true);
	luaInterface.initLua();

	auto mdiArea = LuaIntf::Lua::getGlobal<QMdiArea*>(L, "mdiArea");

	if(mdiArea->subWindowList().count() == 0) {
		LuaIntf::LuaRef(L, "new_file")();
	}

	mdiArea->setActiveSubWindow(mdiArea->subWindowList().at(0));

	auto mdiChild = dynamic_cast<CadMdiChild*>(mdiArea->activeSubWindow()->widget());
	auto storageManager = mdiChild->storageManager();
	auto documentCanvas = mdiChild->viewer()->documentCanvas();

	lc::entity::Line_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();

	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());



	LuaIntf::LuaRef(L, "create_line")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "Line was not created";



	documentCanvas->makeSelection(0, 0, 100, 100, false, false);
	documentCanvas->closeSelection();

	EXPECT_EQ(1, mdiChild->selection().size()) << "Entity not selected";



	LuaIntf::LuaRef(L, "rotate_selected_entities")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 100));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 0));

	entities = storageManager->entityContainer().asVector();
	createdEntity = std::dynamic_pointer_cast<const lc::entity::Line>(entities.at(0));

	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->start());
	EXPECT_EQ(lc::geo::Coordinate(100, -100), createdEntity->end());
}

TEST(LuaUIOperationsTest, Copy) {
	QApplication app(argc, argv);
	LuaInterface luaInterface;
	auto L = luaInterface.luaState();

	luaInterface.hideUI(true);
	luaInterface.initLua();

	auto mdiArea = LuaIntf::Lua::getGlobal<QMdiArea*>(L, "mdiArea");

	if(mdiArea->subWindowList().count() == 0) {
		LuaIntf::LuaRef(L, "new_file")();
	}

	mdiArea->setActiveSubWindow(mdiArea->subWindowList().at(0));

	auto mdiChild = dynamic_cast<CadMdiChild*>(mdiArea->activeSubWindow()->widget());
	auto storageManager = mdiChild->storageManager();
	auto documentCanvas = mdiChild->viewer()->documentCanvas();

	lc::entity::Line_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();

	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());



	LuaIntf::LuaRef(L, "create_line")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "Line was not created";



	documentCanvas->makeSelection(0, 0, 100, 100, false, false);
	documentCanvas->closeSelection();

	EXPECT_EQ(1, mdiChild->selection().size()) << "Entity not selected";



	LuaIntf::LuaRef(L, "copy_selected_entities")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));

	entities = storageManager->entityContainer().asVector();

	ASSERT_EQ(2, entities.size());

	createdEntity = std::dynamic_pointer_cast<const lc::entity::Line>(entities.at(0));
	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->start());
	EXPECT_EQ(lc::geo::Coordinate(100, 100), createdEntity->end());

	createdEntity = std::dynamic_pointer_cast<const lc::entity::Line>(entities.at(1));
	EXPECT_EQ(lc::geo::Coordinate(100, 100), createdEntity->start());
	EXPECT_EQ(lc::geo::Coordinate(200, 200), createdEntity->end());
}

TEST(LuaUIOperationsTest, Scale) {
	QApplication app(argc, argv);
	LuaInterface luaInterface;
	auto L = luaInterface.luaState();

	luaInterface.hideUI(true);
	luaInterface.initLua();

	auto mdiArea = LuaIntf::Lua::getGlobal<QMdiArea*>(L, "mdiArea");

	if(mdiArea->subWindowList().count() == 0) {
		LuaIntf::LuaRef(L, "new_file")();
	}

	mdiArea->setActiveSubWindow(mdiArea->subWindowList().at(0));

	auto mdiChild = dynamic_cast<CadMdiChild*>(mdiArea->activeSubWindow()->widget());
	auto storageManager = mdiChild->storageManager();
	auto documentCanvas = mdiChild->viewer()->documentCanvas();

	lc::entity::Line_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();

	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());



	LuaIntf::LuaRef(L, "create_line")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "Line was not created";



	documentCanvas->makeSelection(0, 0, 100, 100, false, false);
	documentCanvas->closeSelection();

	EXPECT_EQ(1, mdiChild->selection().size()) << "Entity not selected";



	LuaIntf::LuaRef(L, "scale_selected_entities")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "number")(2);

	entities = storageManager->entityContainer().asVector();

	createdEntity = std::dynamic_pointer_cast<const lc::entity::Line>(entities.at(0));
	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->start());
	EXPECT_EQ(lc::geo::Coordinate(200, 200), createdEntity->end());
}

TEST(LuaUIOperationsTest, Remove) {
	QApplication app(argc, argv);
	LuaInterface luaInterface;
	auto L = luaInterface.luaState();

	luaInterface.hideUI(true);
	luaInterface.initLua();

	auto mdiArea = LuaIntf::Lua::getGlobal<QMdiArea*>(L, "mdiArea");

	if(mdiArea->subWindowList().count() == 0) {
		LuaIntf::LuaRef(L, "new_file")();
	}

	mdiArea->setActiveSubWindow(mdiArea->subWindowList().at(0));

	auto mdiChild = dynamic_cast<CadMdiChild*>(mdiArea->activeSubWindow()->widget());
	auto storageManager = mdiChild->storageManager();
	auto documentCanvas = mdiChild->viewer()->documentCanvas();

	lc::entity::Line_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();

	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());



	LuaIntf::LuaRef(L, "create_line")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "Line was not created";



	documentCanvas->makeSelection(0, 0, 100, 100, false, false);
	documentCanvas->closeSelection();

	EXPECT_EQ(1, mdiChild->selection().size()) << "Entity not selected";



	LuaIntf::LuaRef(L, "remove_selected_entities")();

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());
}