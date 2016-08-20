#include <gtest/gtest.h>
#include "cad/geometry/geobezier.h"
#include "cad/geometry/geobezierbase.h"
#include "cad/geometry/geobeziercubic.h"

#include "cad/math/intersectionhandler.h"
#include "cad/geometry/geovector.h"
#include <cmath>

#define PI 3.14159265
#define TRD PI/180

TEST(BEZIER_QUADRATIC, LENGTH) {
    auto p1 = lc::geo::Coordinate(50,230);
    auto p2 = lc::geo::Coordinate(50,50);
    auto p3 = lc::geo::Coordinate(230, 50);

    auto bb = lc::geo::Bezier(p1,p2,p3);
    auto len = bb.length();
    ASSERT_NEAR(292.180, len, 1e-2);
}

TEST(BEZIER_QUADRATIC, NORMAL) {
    auto p1 = lc::geo::Coordinate(50,230);
    auto p2 = lc::geo::Coordinate(50,50);
    auto p3 = lc::geo::Coordinate(230, 50);

std::vector<double> values = {-90, -96.3402, -104.036, -113.199, -123.69,
                            -135, -146.31, -156.801, -165.964, -173.66,
                             -180 };


    auto z= 0;
    auto bb = lc::geo::Bezier(p1,p2,p3);
    for(double i = 0.; i < 1.; i = i + 0.1) {
        auto start = bb.DirectValueAt(i);
        auto end = bb.normal(i);
        ASSERT_NEAR(atan2 (end.x(),end.y()) * 180 / PI, values.at(z), 1e-2);
        z++;
    }

}

TEST(BEZIER_QUADRATIC, LINE) {
    auto c1 = lc::geo::Coordinate(50,50);
    auto c2 = lc::geo::Coordinate(1000,1000);
    auto line_ = lc::geo::Vector(c1,c2);

    auto p1 = lc::geo::Coordinate(100,-50);
    auto p2 = lc::geo::Coordinate(0,500);
    auto p3 = lc::geo::Coordinate(400,-200);
    auto ret_size = 2;
    auto bezier_ = std::make_shared<lc::geo::Bezier>(lc::geo::Bezier(p1,p2,p3));

    ASSERT_EQ(ret_size, lc::maths::Intersection::bezierLine(bezier_, line_).size());


    c1 = lc::geo::Coordinate(0,0);
    c2 = lc::geo::Coordinate(500,0);
    line_ = lc::geo::Vector(c1,c2);

    p1 = lc::geo::Coordinate(100, 50);
    p2 = lc::geo::Coordinate(240,0);
    p3 = lc::geo::Coordinate(300,-200);
    ret_size = 1;
    auto bezier_2 = std::make_shared<lc::geo::Bezier>(lc::geo::Bezier(p1,p2,p3));

    ASSERT_EQ(ret_size, lc::maths::Intersection::bezierLine(bezier_2, line_).size());

}

TEST(BEZIER_QAUDRATIC, CIRCLE) {
    auto c1 = lc::geo::Coordinate(0,0);
    auto circle_ = lc::geo::Circle(c1, 200.);

    auto p1 = lc::geo::Coordinate(200,300);
    auto p2 = lc::geo::Coordinate(0,0);
    auto p3 = lc::geo::Coordinate(-200,300);

    auto bezier_ = std::make_shared<lc::geo::Bezier>(lc::geo::Bezier(p1,p2,p3));

    auto ret = lc::maths::Intersection::bezierCircle(bezier_, circle_);
    auto ret_size = 2;
    ASSERT_EQ(ret_size, ret.size());


    c1 = lc::geo::Coordinate(0,0);
    auto circle_2 = lc::geo::Circle(c1, 200.);

    p1 = lc::geo::Coordinate(0,5);
    p2 = lc::geo::Coordinate(200,0);
    p3 = lc::geo::Coordinate(400,0);

    auto bezier_2 = std::make_shared<lc::geo::Bezier>(lc::geo::Bezier(p1,p2,p3));

    ret = lc::maths::Intersection::bezierCircle(bezier_2, circle_2);
    ret_size = 1;
    ASSERT_EQ(ret_size, ret.size());
}

TEST(BEZIER_QAUDRATIC, ARC) {
    auto c1 = lc::geo::Coordinate(0,0);
    auto arc_ = lc::geo::Arc(c1, 200., 90*TRD, 270*TRD);

    auto p1 = lc::geo::Coordinate(200,300);
    auto p2 = lc::geo::Coordinate(0,0);
    auto p3 = lc::geo::Coordinate(-200,300);

    auto bezier_ = std::make_shared<lc::geo::Bezier>(lc::geo::Bezier(p1,p2,p3));

    auto ret = lc::maths::Intersection::bezierArc(bezier_, arc_);

    auto ret_size = 1;
    ASSERT_EQ(ret_size, ret.size());


    c1 = lc::geo::Coordinate(0,0);
    arc_ = lc::geo::Arc(c1, 200., 0, PI);

    p1 = lc::geo::Coordinate(200,300);
    p2 = lc::geo::Coordinate(0,0);
    p3 = lc::geo::Coordinate(-200,300);

    auto bezier_2 = std::make_shared<lc::geo::Bezier>(lc::geo::Bezier(p1,p2,p3));

    ret = lc::maths::Intersection::bezierArc(bezier_2, arc_);
    ret_size = 2;
    ASSERT_EQ(ret_size, ret.size());


    c1 = lc::geo::Coordinate(0,0);
    arc_ = lc::geo::Arc(c1, 200., PI, 2*PI);

    p1 = lc::geo::Coordinate(200,300);
    p2 = lc::geo::Coordinate(0,0);
    p3 = lc::geo::Coordinate(-200,300);

    auto bezier_3 = std::make_shared<lc::geo::Bezier>(lc::geo::Bezier(p1,p2,p3));

    ret = lc::maths::Intersection::bezierArc(bezier_3, arc_);
    ret_size = 0;
    ASSERT_EQ(ret_size, ret.size());

}

TEST(BEZIER_CUBIC, LINE) {
    auto c1 = lc::geo::Coordinate(-1000,0);
    auto c2 = lc::geo::Coordinate(1000,0);
    auto line_ = lc::geo::Vector(c1,c2);

    auto p1 = lc::geo::Coordinate(100,-500);
    auto p2 = lc::geo::Coordinate(300,1500);
    auto p3 = lc::geo::Coordinate(600,-1500);
    auto p4 = lc::geo::Coordinate(1000,500);

    auto ret_size = 3;
    auto bezier_ = std::make_shared<lc::geo::CubicBezier>(lc::geo::CubicBezier(p1,p2,p3,p4));

    ASSERT_EQ(ret_size, lc::maths::Intersection::bezierLine(bezier_, line_).size());


    c1 = lc::geo::Coordinate(-1000,0);
    c2 = lc::geo::Coordinate(1000,0);
    line_ = lc::geo::Vector(c1,c2);

    p1 = lc::geo::Coordinate(100,-500);
    p2 = lc::geo::Coordinate(300,500);
    p3 = lc::geo::Coordinate(600,-500);
    p4 = lc::geo::Coordinate(1000,500);

    ret_size = 1;

    auto bezier_2 = std::make_shared<lc::geo::CubicBezier>(lc::geo::CubicBezier(p1,p2,p3,p4));

    ASSERT_EQ(ret_size, lc::maths::Intersection::bezierLine(bezier_2, line_).size());

}


TEST(BEZIER_CUBIC, CIRCLE) {
    auto c1 = lc::geo::Coordinate(200,0);
    auto circle_ = lc::geo::Circle(c1, 100.);

    auto p1 = lc::geo::Coordinate(100,-500);
    auto p2 = lc::geo::Coordinate(300,1500);
    auto p3 = lc::geo::Coordinate(600,-1500);
    auto p4 = lc::geo::Coordinate(1000,500);

    auto bezier_ = std::make_shared<lc::geo::CubicBezier>(lc::geo::CubicBezier(p1,p2,p3,p4));

    auto ret = lc::maths::Intersection::bezierCircle(bezier_, circle_);
    auto ret_size = 6;
    ASSERT_EQ(ret_size, ret.size());

}

//TEST(BEZIER_CUBIC, ARC) {
//    auto c1 = lc::geo::Coordinate(0,0);
//    auto arc_ = lc::geo::Arc(c1, 200., 90*TRD, 270*TRD);

//    auto p1 = lc::geo::Coordinate(200,300);
//    auto p2 = lc::geo::Coordinate(0,0);
//    auto p3 = lc::geo::Coordinate(-200,300);
//    auto p4 = lc::geo::Coordinate(400,300);

//    auto bezier_ = std::make_shared<lc::geo::CubicBezier>(lc::geo::Bezier(p1,p2,p3,p4));

//    auto ret = lc::maths::Intersection::bezierArc(bezier_, arc_);

//    auto ret_size = 1;
//    ASSERT_EQ(ret_size, ret.size());


//    c1 = lc::geo::Coordinate(0,0);
//    arc_ = lc::geo::Arc(c1, 200., 0, PI);

//    p1 = lc::geo::Coordinate(200,300);
//    p2 = lc::geo::Coordinate(0,0);
//    p3 = lc::geo::Coordinate(-200,300);
//    p4 = lc::geo::Coordinate(400,300);

//    auto bezier_ = std::make_shared<lc::geo::CubicBezier>(lc::geo::Bezier(p1,p2,p3,p4));

//    ret = lc::maths::Intersection::bezierArc(bezier_2, arc_);
//    ret_size = 2;
//    ASSERT_EQ(ret_size, ret.size());


//    c1 = lc::geo::Coordinate(0,0);
//    arc_ = lc::geo::Arc(c1, 200., PI, 2*PI);

//    p1 = lc::geo::Coordinate(200,300);
//    p2 = lc::geo::Coordinate(0,0);
//    p3 = lc::geo::Coordinate(-200,300);
//    p4 = lc::geo::Coordinate(400,300);

//    auto bezier_ = std::make_shared<lc::geo::CubicBezier>(lc::geo::Bezier(p1,p2,p3,p4));

//    ret = lc::maths::Intersection::bezierArc(bezier_3, arc_);
//    ret_size = 0;
//    ASSERT_EQ(ret_size, ret.size());

//}

