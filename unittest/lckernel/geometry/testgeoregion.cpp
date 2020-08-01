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
}
