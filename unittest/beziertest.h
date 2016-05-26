#include "cad/geometry/geobezier.h"

TEST(BEZIER, BOUNDINGBOX) {
    auto p1 = lc::geo::Coordinate(180,240);
    auto p2 = lc::geo::Coordinate(2,2);
    auto p3 = lc::geo::Coordinate(220, 180);

    auto bb = lc::geo::Bezier(p1,p2,p3);
    auto area = bb.boundingBox();
    std::cout << area.minP() << "\t\t" << area.maxP() << "\n";
}
