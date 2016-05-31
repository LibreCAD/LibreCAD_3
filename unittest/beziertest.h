#include "cad/geometry/geobezier.h"
#include <cmath>
TEST(BEZIER, LENGTH) {
    auto p1 = lc::geo::Coordinate(50,230);
    auto p2 = lc::geo::Coordinate(50,50);
    auto p3 = lc::geo::Coordinate(230, 50);

    auto bb = lc::geo::Bezier(p1,p2,p3);
    auto len = bb.length();
    ASSERT_NEAR(292.180, len, 1e-2);
}

#define PI 3.14159265

TEST(BEZIER, NORMAL) {
    auto p1 = lc::geo::Coordinate(50,230);
    auto p2 = lc::geo::Coordinate(50,50);
    auto p3 = lc::geo::Coordinate(230, 50);

    auto bb = lc::geo::Bezier(p1,p2,p3);
    for(double i = 0.; i < 1.; i = i + 0.1) {
        auto start = bb.DirectValueAt(i);
        auto end = bb.normal(i);
        std::cout << atan2 (end.x(),end.y()) * 180 / PI << std::endl;
    }

}
