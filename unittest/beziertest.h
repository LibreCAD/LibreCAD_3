#include "cad/geometry/geobezier.h"
#include "cad/math/intersectionhandler.h"
#include "cad/geometry/geovector.h"
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

//TEST(BEZIER, LINE) {
//    auto c1 = lc::geo::Coordinate(50,50);
//    auto c2 = lc::geo::Coordinate(1000,1000);
//    auto line_ = lc::geo::Vector(c1,c2);

//    auto p1 = lc::geo::Coordinate(100,-50);
//    auto p2 = lc::geo::Coordinate(0,500);
//    auto p3 = lc::geo::Coordinate(400,-200);

//    auto bezier_ = lc::geo::Bezier(p1,p2,p3);

//    lc::maths::Intersection::BezierLine(bezier_, line_);
//}

TEST(BEZIER, CIRCLE) {
    auto c1 = lc::geo::Coordinate(0,0);
    auto circle_ = lc::geo::Circle(c1, 200.);

    auto p1 = lc::geo::Coordinate(200,300);
    auto p2 = lc::geo::Coordinate(0,0);
    auto p3 = lc::geo::Coordinate(-200,300);

    auto bezier_ = lc::geo::Bezier(p1,p2,p3);

    auto ret = lc::maths::Intersection::BezierCircle(bezier_, circle_);

    std::cout << ret.size();

    for(auto r : ret) {
        std::cout << r << std::endl;
    }
}
