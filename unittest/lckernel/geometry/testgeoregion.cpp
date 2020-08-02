#include <gtest/gtest.h>
#include <cad/geometry/georegion.h>

using namespace lc;
using namespace geo;

TEST(lc__geo__RegionTest, creation) {
	std::vector<lc::entity::CADEntity_CSPtr> loopData;
	//Create a circle
	auto arc= std::make_shared<lc::entity::Arc>(geo::Coordinate(0,0), 100, 0, 2*M_PI, true, nullptr, nullptr, nullptr);
	loopData.push_back(arc);
	Loop loop(loopData);
	Region reg;
	reg.addLoop(loop);
	ASSERT_EQ(reg.numLoops(), 1);
	auto bbox = reg.boundingBox();
	//display BBOX
	std::cout << bbox << std::endl;
	//Check for point in and out
	// (10,10) should be in but (95,95) should be out.
	ASSERT_TRUE(reg.isPointInside(lc::geo::Coordinate(10, 10)));
	ASSERT_FALSE(reg.isPointInside(lc::geo::Coordinate(95, 95)));
	ASSERT_FALSE(reg.isPointInside(lc::geo::Coordinate(1000, 1000)));
}
