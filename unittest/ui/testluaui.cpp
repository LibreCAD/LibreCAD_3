#include <gtest/gtest.h>

#include <QApplication>

#include <luainterface.h>
#include "uitests.h"

TEST(LuaUITest, NewFile) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
	auto L = luaInterface->luaState();

	auto mdiArea = LuaIntf::Lua::getGlobal<QMdiArea*>(L, "mdiArea");
	int nbOpenWindow = mdiArea->subWindowList().count();

	LuaIntf::LuaRef(L, "new_file")();

	EXPECT_EQ(nbOpenWindow + 1, mdiArea->subWindowList().count());
    delete luaInterface;
}

TEST(LuaUITest, CloseWindow) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

	auto mdiArea = LuaIntf::Lua::getGlobal<QMdiArea*>(L, "mdiArea");

	if(mdiArea->subWindowList().count() == 0) {
		LuaIntf::LuaRef(L, "new_file")();
	}

	int nbOpenWindow = mdiArea->subWindowList().count();

	mdiArea->subWindowList().at(0)->close();

	EXPECT_EQ(nbOpenWindow - 1, mdiArea->subWindowList().count());
    delete luaInterface;
}

TEST(LuaUITest, UndoRedo) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

	auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();
	auto nbEntities = storageManager->entityContainer().asVector().size();

	LuaIntf::LuaRef(L, "create_line")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));

	EXPECT_EQ(nbEntities + 1, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "undo")();

	EXPECT_EQ(nbEntities, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "redo")();

	EXPECT_EQ(nbEntities + 1, storageManager->entityContainer().asVector().size());
    delete luaInterface;
}

TEST(LuaUITest, LineCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::Line_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;



	//2 points line
	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_line")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "2 points line was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Line>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->start());
	EXPECT_EQ(lc::geo::Coordinate(100, 100), createdEntity->end());



	//2 points + length line
	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_line")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "number")(100);
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(200, 0));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "2 points + length line was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Line>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->start());
	EXPECT_EQ(lc::geo::Coordinate(100, 0), createdEntity->end());
    delete luaInterface;
}

TEST(LuaUITest, CircleCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::Circle_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;



	//2 points circle
	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_circle")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 0));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "2 points circle was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Circle>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->center());
	EXPECT_EQ(100, createdEntity->radius());



	//Center + radius circle
	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_circle")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));
	LuaIntf::LuaRef(L, "number")(100);

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "2 points circle was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Circle>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(100, 100), createdEntity->center());
	EXPECT_EQ(100, createdEntity->radius());
    delete luaInterface;
}

TEST(LuaUITest, ArcCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

	auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::Arc_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;



	//4 points arc
	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_arc")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 100));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 0));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "4 points arc was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Arc>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->center());
	EXPECT_EQ(100, createdEntity->radius());
	EXPECT_EQ(0.5 * M_PI, createdEntity->startAngle());
	EXPECT_EQ(0, createdEntity->endAngle());



	//Center + radius + 2 angle arc
	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_arc")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));
	LuaIntf::LuaRef(L, "number")(100);
	LuaIntf::LuaRef(L, "number")(0);
	LuaIntf::LuaRef(L, "number")(1 * M_PI);

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "Center + radius + 2 angle arc was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Arc>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(100, 100), createdEntity->center());
	EXPECT_EQ(100, createdEntity->radius());
	EXPECT_EQ(0, createdEntity->startAngle());
	EXPECT_EQ(1 * M_PI, createdEntity->endAngle());
    delete luaInterface;
}

TEST(LuaUITest, EllipseCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::Ellipse_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;



	//3 points ellipse
	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_ellipse")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 10));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "3 points ellipse was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Ellipse>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->center());
	EXPECT_EQ(100, createdEntity->majorRadius());
	EXPECT_EQ(10, createdEntity->minorRadius());



	//2 points + radius ellipse
	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_ellipse")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 0));
	LuaIntf::LuaRef(L, "number")(10);

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "3 points ellipse was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Ellipse>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(100, 100), createdEntity->center());
	EXPECT_EQ(100, createdEntity->majorRadius());
	EXPECT_EQ(10, createdEntity->minorRadius());
    delete luaInterface;
}

TEST(LuaUITest, ArcEllipseCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::Ellipse_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;



	//5 points ellipse
	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_arc_ellipse")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 10));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(1, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 1));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "5 points ellipse was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Ellipse>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->center());
	EXPECT_EQ(100, createdEntity->majorRadius());
	EXPECT_EQ(10, createdEntity->minorRadius());
	EXPECT_EQ(0, createdEntity->startAngle());
	EXPECT_EQ(0.5 * M_PI, createdEntity->endAngle());



	//2 points + radius + 2 angle ellipse
	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_arc_ellipse")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 0));
	LuaIntf::LuaRef(L, "number")(10);
	LuaIntf::LuaRef(L, "number")(0);
	LuaIntf::LuaRef(L, "number")(1 * M_PI);

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "2 points + radius + 2 angle ellipse was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Ellipse>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(100, 100), createdEntity->center());
	EXPECT_EQ(100, createdEntity->majorRadius());
	EXPECT_EQ(10, createdEntity->minorRadius());
	EXPECT_EQ(0, createdEntity->startAngle());
	EXPECT_EQ(1 * M_PI, createdEntity->endAngle());
    delete luaInterface;
}

TEST(LuaUITest, DimAlignedCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();;

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::DimAligned_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_dim_aligned")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(50, 100));
	LuaIntf::LuaRef(L, "text")(QString("Test: <>"));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "DimAligned was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::DimAligned>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->definitionPoint2());
	EXPECT_EQ(lc::geo::Coordinate(100, 0), createdEntity->definitionPoint3());
	EXPECT_EQ(lc::geo::Coordinate(50, 100), createdEntity->middleOfText());
	EXPECT_EQ("Test: <>", createdEntity->explicitValue());
    delete luaInterface;
}

TEST(LuaUITest, DimAngularCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::DimAngular_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_dim_angular")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 0));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 100));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));
	LuaIntf::LuaRef(L, "text")(QString("Test: <>"));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "DimAngular was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::DimAngular>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 0), createdEntity->defLine11());
	EXPECT_EQ(lc::geo::Coordinate(0, 100), createdEntity->defLine12());
	EXPECT_EQ(lc::geo::Coordinate(100, 100), createdEntity->defLine22());
	EXPECT_EQ("Test: <>", createdEntity->explicitValue());
    delete luaInterface;
}

TEST(LuaUITest, DimDiametricCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::DimDiametric_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_dim_diametric")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 10));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 10));
	LuaIntf::LuaRef(L, "text")(QString("Test: <>"));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "DimDiametric was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::DimDiametric>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 10), createdEntity->definitionPoint());
	EXPECT_EQ(lc::geo::Coordinate(100, 10), createdEntity->definitionPoint2());
	EXPECT_EQ("Test: <>", createdEntity->explicitValue());
    delete luaInterface;
}

TEST(LuaUITest, DimLinearCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::DimLinear_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_dim_linear")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 10));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 10));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(50, 100));
	LuaIntf::LuaRef(L, "text")(QString("Test: <>"));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "DimLinear was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::DimLinear>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 10), createdEntity->definitionPoint2());
	EXPECT_EQ(lc::geo::Coordinate(100, 10), createdEntity->definitionPoint3());
	EXPECT_EQ(lc::geo::Coordinate(50, 100), createdEntity->middleOfText());
	EXPECT_EQ("Test: <>", createdEntity->explicitValue());
    delete luaInterface;
}

TEST(LuaUITest, DimRadialCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::DimRadial_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_dim_radial")();
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 10));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 10));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(50, 100));
	LuaIntf::LuaRef(L, "text")(QString("Test: <>"));

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "DimRadial was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::DimRadial>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(lc::geo::Coordinate(0, 10), createdEntity->definitionPoint());
	EXPECT_EQ(lc::geo::Coordinate(100, 10), createdEntity->definitionPoint2());
	EXPECT_EQ(lc::geo::Coordinate(50, 100), createdEntity->middleOfText());
	EXPECT_EQ("Test: <>", createdEntity->explicitValue());
    delete luaInterface;
}

TEST(LuaUITest, SplineCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::Spline_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	std::vector<lc::geo::Coordinate> points = {
			lc::geo::Coordinate(0, 0),
			lc::geo::Coordinate(10, 10),
			lc::geo::Coordinate(20, 0),
			lc::geo::Coordinate(30, 30),
			lc::geo::Coordinate(100, -100)
	};

	LuaIntf::LuaRef(L, "create_spline")();

	for(auto point : points) {
		LuaIntf::LuaRef(L, "event.trigger")("point", point);
	}

	LuaIntf::LuaRef(L, "finish_operation")();

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "Spline was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::Spline>(*storageManager->entityContainer().asVector().begin());

	EXPECT_EQ(points, createdEntity->controlPoints());
    delete luaInterface;
}

TEST(LuaUITest, LWPolylineCreation) {
	QApplication app(argc, argv);
	auto luaInterface = startLC();
    auto L = luaInterface->luaState();

    auto mdiChild = getMdiChild(L);
    
	auto storageManager = mdiChild->storageManager();

	lc::entity::LWPolyline_CSPtr createdEntity;
	std::vector<lc::entity::CADEntity_CSPtr> entities;

	entities = storageManager->entityContainer().asVector();
	for(auto entity : entities) {
		storageManager->removeEntity(entity);
	}

	EXPECT_EQ(0, storageManager->entityContainer().asVector().size());

	LuaIntf::LuaRef(L, "create_lw_polyline")();

	L.doString("op[active_widget().id]:createLine()");
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(0, 10));
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 10));

	L.doString("op[active_widget().id]:createArc()");
	LuaIntf::LuaRef(L, "number")(1 * M_PI);
	LuaIntf::LuaRef(L, "event.trigger")("point", lc::geo::Coordinate(100, 100));

	LuaIntf::LuaRef(L, "finish_operation")();

	EXPECT_EQ(1, storageManager->entityContainer().asVector().size()) << "LWPolyline was not created";

	createdEntity = std::static_pointer_cast<const lc::entity::LWPolyline>(*storageManager->entityContainer().asVector().begin());
	auto vertex = createdEntity->vertex();

	EXPECT_EQ(3, vertex.size());

	EXPECT_EQ(lc::geo::Coordinate(0, 10), vertex.at(0).location());
	EXPECT_EQ(0, vertex.at(0).bulge());

	EXPECT_EQ(lc::geo::Coordinate(100, 10), vertex.at(1).location());
	EXPECT_FLOAT_EQ(1, vertex.at(1).bulge());

	EXPECT_EQ(lc::geo::Coordinate(100, 100), vertex.at(2).location());
	EXPECT_FLOAT_EQ(1, vertex.at(2).bulge());
    delete luaInterface;
}