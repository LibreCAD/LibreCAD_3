#include <gtest/gtest.h>
#include "cad/geometry/geobezier.h"
#include "cad/geometry/geobezierbase.h"
#include "cad/geometry/geobeziercubic.h"
#include "cad/geometry/geospline.h"
#include <limits>

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
                                  -180
                                 };

    auto z = 0;
    auto bb = lc::geo::Bezier(p1,p2,p3);
    for(double i = 0.; i < 1.; i = i + 0.1) {
        //auto start = bb.DirectValueAt(i);
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

TEST(SPLINE, SPLINELINE) {
    using namespace lc;

    auto p1 = geo::Coordinate(87.74071358645254, -38.85660173114327);
    auto p2 = geo::Coordinate(206.8173963109239, 139.1317029728033);

    auto line = geo::Vector(p1,p2);
    std::vector<geo::Coordinate> cp = { {52.25, 57.25},
        {114.75, -4.5}, {174.5, 19.75}, {193.25, 77}, {239.75, 54}
    };

    auto degree = 2;
    auto spline_ = geo::Spline(cp, {}, {}, degree, false, 0, 0,0,0, 0,0,0, 0,0,0, static_cast<geo::Spline::splineflag>(1));

    auto expectedres = 1;
    auto result = lc::maths::Intersection::splineLine(spline_, line).size();

    ASSERT_EQ(result, expectedres);
}

TEST(SplineBezierConversion, UsesNonuniformKnotVector) {
    const std::vector<lc::geo::Coordinate> controls{{0,0,3}, {1,2,3}, {3,2,3}, {4,0,3}};
    const std::vector<double> knots{0,0,0,0.25,1,1,1};
    const lc::geo::Spline spline(controls, knots, {}, 2, false, 0.0,
                                 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::PLANAR);
    const auto segments = spline.beziers();
    ASSERT_EQ(segments.size(), 2u);
    ASSERT_NE(segments[0], nullptr);
    ASSERT_NE(segments[1], nullptr);
    const auto first = segments[0]->getCP();
    const auto second = segments[1]->getCP();
    ASSERT_EQ(first.size(), 3u);
    ASSERT_EQ(second.size(), 3u);
    // Quadratic de Boor evaluation at u=0.25: 0.75 * P1 + 0.25 * P2.
    EXPECT_NEAR(first.back().x(), 1.5, 1e-12);
    EXPECT_NEAR(first.back().y(), 2.0, 1e-12);
    EXPECT_NEAR(first.back().z(), 3.0, 1e-12);
    EXPECT_NEAR(second.front().x(), 1.5, 1e-12);
    EXPECT_NEAR(second.front().y(), 2.0, 1e-12);
    EXPECT_NEAR(second.front().z(), 3.0, 1e-12);
}

TEST(SplineBezierConversion, UsesCubicNonuniformKnotVector) {
    const std::vector<lc::geo::Coordinate> controls{{0,0,2}, {1,3,2}, {2,-1,2}, {4,2,2}, {5,0,2}};
    const std::vector<double> knots{0,0,0,0,0.4,1,1,1,1};
    const lc::geo::Spline spline(controls, knots, {}, 3, false, 0.0,
                                 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::PLANAR);
    const auto segments = spline.beziers();
    ASSERT_EQ(segments.size(), 2u);
    const auto first = segments[0]->getCP();
    const auto second = segments[1]->getCP();
    ASSERT_EQ(first.size(), 4u);
    ASSERT_EQ(second.size(), 4u);
    // Independently evaluated de Boor point at u=0.4.
    EXPECT_NEAR(first.back().x(), 1.96, 1e-12);
    EXPECT_NEAR(first.back().y(), 0.92, 1e-12);
    EXPECT_NEAR(first.back().z(), 2.0, 1e-12);
    EXPECT_NEAR(second.front().x(), 1.96, 1e-12);
    EXPECT_NEAR(second.front().y(), 0.92, 1e-12);
    EXPECT_NEAR(second.front().z(), 2.0, 1e-12);
}

TEST(SplineBezierConversion, MissingOrInvalidKnotsKeepExistingFallback) {
    const std::vector<lc::geo::Coordinate> controls{{0,0,3}, {1,2,3}, {3,2,3}, {4,0,3}};
    for (const auto& knots : {std::vector<double>{},
                              std::vector<double>{0,0,0,0.5,0.25,1,1},
                              std::vector<double>{0,0,0,0.5,1,1},
                              std::vector<double>{0,0,0,std::numeric_limits<double>::quiet_NaN(),1,1,1},
                              std::vector<double>{0,0,0,0.00001,1,1,1},
                              std::vector<double>{0,0,0,0,1,1,1},
                              std::vector<double>{-1e308,-1e308,-1e308,0,1e308,1e308,1e308}}) {
        const lc::geo::Spline spline(controls, knots, {}, 2, false, 0.0,
                                     0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::PLANAR);
        const auto segments = spline.beziers();
        ASSERT_EQ(segments.size(), 2u);
        const auto first = segments[0]->getCP();
        ASSERT_EQ(first.size(), 3u);
        EXPECT_NEAR(first.back().x(), 2.0, 1e-12);
    }
}

TEST(SplineBezierConversion, SupportsShiftedKnotDomain) {
    const std::vector<lc::geo::Coordinate> controls{{0,0,3}, {1,2,3}, {3,2,3}, {4,0,3}};
    const std::vector<double> knots{10,10,10,11,14,14,14};
    const lc::geo::Spline spline(controls, knots, {}, 2, false, 0.0,
                                 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::PLANAR);
    const auto segments = spline.beziers();
    ASSERT_EQ(segments.size(), 2u);
    EXPECT_NEAR(segments[0]->getCP().back().x(), 1.5, 1e-12);
}

TEST(SplineBezierConversion, DegreeOneProducesExactLinearSegments) {
    const std::vector<lc::geo::Coordinate> controls{{0,0,3}, {2,4,5}, {6,2,7}};
    const std::vector<double> knots{0,0,0.25,1,1};
    const lc::geo::Spline spline(controls, knots, {}, 1, false, 0.0,
                                 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::LINEAR);
    const auto segments = spline.beziers();
    ASSERT_EQ(segments.size(), 2u);
    for (std::size_t i = 0; i < segments.size(); ++i) {
        const auto cps = segments[i]->getCP();
        ASSERT_EQ(cps.size(), 3u);
        EXPECT_NEAR(cps[0].x(), controls[i].x(), 1e-12);
        EXPECT_NEAR(cps[0].y(), controls[i].y(), 1e-12);
        EXPECT_NEAR(cps[0].z(), controls[i].z(), 1e-12);
        EXPECT_NEAR(cps[2].x(), controls[i+1].x(), 1e-12);
        EXPECT_NEAR(cps[2].y(), controls[i+1].y(), 1e-12);
        EXPECT_NEAR(cps[2].z(), controls[i+1].z(), 1e-12);
        const auto midpoint = segments[i]->DirectValueAt(0.5);
        EXPECT_NEAR(midpoint.x(), (controls[i].x() + controls[i+1].x()) / 2, 1e-12);
        EXPECT_NEAR(midpoint.y(), (controls[i].y() + controls[i+1].y()) / 2, 1e-12);
        EXPECT_NEAR(midpoint.z(), (controls[i].z() + controls[i+1].z()) / 2, 1e-12);
    }
}

TEST(SplineBezierConversion, DegreeOneIntersectsCrossingLine) {
    const std::vector<lc::geo::Coordinate> controls{{0,0}, {2,4}, {6,2}};
    const std::vector<double> knots{0,0,0.25,1,1};
    const lc::geo::Spline spline(controls, knots, {}, 1, false, 0.0,
                                 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::LINEAR);
    const auto hits = lc::maths::Intersection::splineLine(spline,
        lc::geo::Vector(lc::geo::Coordinate(1,-1), lc::geo::Coordinate(1,5)));
    ASSERT_EQ(hits.size(), 1u);
    EXPECT_NEAR(hits[0].x(), 1.0, 1e-12);
    EXPECT_NEAR(hits[0].y(), 2.0, 1e-12);
    const auto parallel = lc::maths::Intersection::splineLine(spline,
        lc::geo::Vector(lc::geo::Coordinate(0,10), lc::geo::Coordinate(2,14)));
    EXPECT_TRUE(parallel.empty());
}

TEST(SplineBezierConversion, DegreeOneIntersectsCircleAndArc) {
    const std::vector<lc::geo::Coordinate> controls{{-2,0,3}, {2,0,3}};
    const std::vector<double> knots{0,0,1,1};
    const lc::geo::Spline spline(controls, knots, {}, 1, false, 0.0,
                                 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::LINEAR);
    auto hits = lc::maths::Intersection::splineCircle(spline,
        lc::geo::Circle(lc::geo::Coordinate(0,0), 1));
    ASSERT_EQ(hits.size(), 2u);
    std::sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.x() < b.x(); });
    EXPECT_NEAR(hits[0].x(), -1.0, 1e-10);
    EXPECT_NEAR(hits[1].x(), 1.0, 1e-10);
    EXPECT_NEAR(hits[0].z(), 3.0, 1e-10);
    EXPECT_NEAR(hits[1].z(), 3.0, 1e-10);
    const auto arcHits = lc::maths::Intersection::splineArc(spline,
        lc::geo::Arc(lc::geo::Coordinate(0,0), 1, -M_PI / 2, M_PI / 2));
    ASSERT_EQ(arcHits.size(), 1u);
    EXPECT_NEAR(arcHits[0].x(), 1.0, 1e-10);
}

TEST(SplineBezierConversion, DegreeOneCircleTangencyAndMiss) {
    const std::vector<double> knots{0,0,1,1};
    const auto check = [&](double y) {
        const lc::geo::Spline spline({{-2,y}, {2,y}}, knots, {}, 1, false, 0.0,
                                     0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::LINEAR);
        return lc::maths::Intersection::splineCircle(spline, lc::geo::Circle({0,0}, 1));
    };
    const auto tangent = check(1.0);
    ASSERT_EQ(tangent.size(), 1u);
    EXPECT_NEAR(tangent[0].x(), 0.0, 1e-10);
    EXPECT_NEAR(tangent[0].y(), 1.0, 1e-10);
    EXPECT_TRUE(check(2.0).empty());
}

TEST(SplineBezierConversion, DegreeOneIntersectsTranslatedRotatedEllipse) {
    const std::vector<double> knots{0,0,1,1};
    const lc::geo::Spline horizontal({{3,7}, {7,7}}, knots, {}, 1, false, 0.0,
                                     0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::LINEAR);
    const lc::geo::Ellipse ellipse({5,7}, {0,2}, 1, 0, 2*M_PI);
    auto hits = lc::maths::Intersection::splineEllipse(horizontal, ellipse);
    ASSERT_EQ(hits.size(), 2u);
    std::sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.x() < b.x(); });
    EXPECT_NEAR(hits[0].x(), 4.0, 1e-10);
    EXPECT_NEAR(hits[1].x(), 6.0, 1e-10);
    EXPECT_NEAR(hits[0].y(), 7.0, 1e-10);
    EXPECT_NEAR(hits[1].y(), 7.0, 1e-10);
    const lc::geo::Spline vertical({{5,4}, {5,10}}, knots, {}, 1, false, 0.0,
                                   0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::LINEAR);
    const lc::geo::Ellipse arc({5,7}, {0,2}, 1, -M_PI/4, 3*M_PI/4);
    const auto arcHits = lc::maths::Intersection::splineEllipse(vertical, arc);
    ASSERT_EQ(arcHits.size(), 1u);
    EXPECT_NEAR(arcHits[0].x(), 5.0, 1e-10);
    EXPECT_NEAR(arcHits[0].y(), 9.0, 1e-10);
    const lc::geo::Ellipse reversed({5,7}, {0,2}, 1, -M_PI/4, 3*M_PI/4, true);
    const auto reversedHits = lc::maths::Intersection::splineEllipse(vertical, reversed);
    ASSERT_EQ(reversedHits.size(), 1u);
    EXPECT_NEAR(reversedHits[0].y(), 5.0, 1e-10);
}

TEST(SplineBezierConversion, DegreeOneIntersectsSmallCircle) {
    const std::vector<double> knots{0,0,1,1};
    const lc::geo::Spline spline({{-2e-8,0}, {2e-8,0}}, knots, {}, 1, false, 0.0,
                                 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::LINEAR);
    auto hits = lc::maths::Intersection::splineCircle(spline, lc::geo::Circle({0,0}, 1e-8));
    ASSERT_EQ(hits.size(), 2u);
    std::sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.x() < b.x(); });
    EXPECT_NEAR(hits[0].x(), -1e-8, 1e-18);
    EXPECT_NEAR(hits[1].x(), 1e-8, 1e-18);
}

TEST(SplineBezierConversion, RationalQuarterCircleUsesWeights) {
    const std::vector<lc::geo::Coordinate> controls{{1,0,3}, {1,1,3}, {0,1,3}};
    const std::vector<double> knots{0,0,0,1,1,1};
    const std::vector<double> weights{1,std::sqrt(0.5),1};
    const lc::geo::Spline spline(controls, knots, {}, 2, false, 0.0,
                                 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::RATIONAL, weights);
    const auto segments = spline.beziers();
    ASSERT_GT(segments.size(), 1u);
    for (const auto& segment : segments) {
        for (double t : {0.0, 0.5, 1.0}) {
            const auto p = segment->DirectValueAt(t);
            EXPECT_NEAR(std::hypot(p.x(), p.y()), 1.0, 0.0002);
            EXPECT_NEAR(p.z(), 3.0, 1e-12);
        }
    }
}

TEST(SplineBezierConversion, WeightedNonuniformCurveMatchesIndependentPoints) {
    const std::vector<lc::geo::Coordinate> controls{{0,0,1}, {2,4,2}, {4,-2,3}, {8,1,4}};
    const std::vector<double> knots{0,0,0,0.35,1,1,1};
    const std::vector<double> weights{1,0.4,2,1.5};
    const lc::geo::Spline spline(controls, knots, {}, 2, false, 0.0,
                                 0,0,0, 0,0,0, 0,0,1,
                                 static_cast<lc::geo::Spline::splineflag>(0), weights);
    const std::vector<lc::geo::Coordinate> expected{
        {0.7947882736156353,0.8295331161780675,1.3973941368078178},
        {2.129564193168434,0.96113074204947,2.064782096584217},
        {3.458333333333333,-0.375,2.7291666666666665},
        {4.008714596949892,-1.1437908496732025,2.945533769063181},
        {5.325,-0.832142857142857,3.319642857142857},
        {6.64842320919002,0.014853296261067128,3.660203409672935}};
    const auto segments = spline.beziers();
    ASSERT_GT(segments.size(), 1u);
    for (const auto& point : expected) {
        double nearest = std::numeric_limits<double>::infinity();
        for (const auto& segment : segments) {
            const auto cp = segment->getCP();
            ASSERT_EQ(cp.size(), 3u);
            const auto d = cp[2] - cp[0];
            const auto q = point - cp[0];
            const double denom = d.x()*d.x() + d.y()*d.y() + d.z()*d.z();
            const double t = denom > 0 ? std::max(0.0, std::min(1.0,
                (q.x()*d.x() + q.y()*d.y() + q.z()*d.z()) / denom)) : 0;
            const auto e = point - (cp[0] + d*t);
            nearest = std::min(nearest, std::hypot(std::hypot(e.x(),e.y()),e.z()));
        }
        EXPECT_LE(nearest, 0.00081); // 1e-4 of the 8-unit control extent.
    }
}

TEST(SplineBezierConversion, RationalSplineCrossingLineTracksCircle) {
    const lc::geo::Spline spline({{1,0,3}, {1,1,3}, {0,1,3}}, {0,0,0,1,1,1}, {},
        2, false, 0.0, 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::RATIONAL,
        {1,std::sqrt(0.5),1});
    const auto hits = lc::maths::Intersection::splineLine(spline,
        lc::geo::Vector({-1,0.5}, {2,0.5}));
    ASSERT_EQ(hits.size(), 1u);
    EXPECT_NEAR(hits[0].x(), std::sqrt(0.75), 0.0002);
    EXPECT_NEAR(hits[0].y(), 0.5, 1e-12);
}

TEST(SplineBezierConversion, RationalSplineCrossingAtSegmentJunction) {
    const lc::geo::Spline spline({{1,0,3}, {1,1,3}, {0,1,3}}, {0,0,0,1,1,1}, {},
        2, false, 0.0, 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::RATIONAL,
        {1,std::sqrt(0.5),1});
    const auto segments = spline.beziers();
    ASSERT_GT(segments.size(), 3u);
    const auto junction = segments[2]->getCP().back();
    const auto hits = lc::maths::Intersection::splineLine(spline,
        lc::geo::Vector({-1,junction.y()}, {2,junction.y()}));
    ASSERT_EQ(hits.size(), 1u);
    EXPECT_NEAR(hits[0].x(), junction.x(), 1e-10);
}

TEST(SplineBezierConversion, RationalSplineFindsExactInteriorJunction) {
    const lc::geo::Spline spline({{-1,-1}, {-0.5,-0.5}, {0,0}, {0.5,0.5}, {1,1}},
        {0,0,0,0.5,0.5,1,1,1}, {}, 2, false, 0.0, 0,0,0, 0,0,0, 0,0,1,
        lc::geo::Spline::RATIONAL, {1,1,1,1,1});
    const auto hits = lc::maths::Intersection::splineLine(spline, lc::geo::Vector({-2,0}, {2,0}));
    ASSERT_EQ(hits.size(), 1u);
    EXPECT_NEAR(hits[0].x(), 0.0, 1e-12);
}

TEST(SplineBezierConversion, InvalidRationalWeightsKeepExistingFallback) {
    const std::vector<lc::geo::Coordinate> controls{{1,0,3}, {1,1,3}, {0,1,3}};
    const std::vector<double> knots{0,0,0,1,1,1};
    for (const auto& weights : {std::vector<double>{1,1}, std::vector<double>{1,0,1},
                                std::vector<double>{1,-1,1},
                                std::vector<double>{1,std::numeric_limits<double>::quiet_NaN(),1}}) {
        const lc::geo::Spline spline(controls, knots, {}, 2, false, 0.0,
                                     0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::RATIONAL, weights);
        ASSERT_EQ(spline.beziers().size(), 1u);
    }
}

TEST(SplineBezierConversion, RationalCubicIncludesIndependentMidpoint) {
    const lc::geo::Spline spline({{0,0,0},{1,4,1},{3,-2,2},{6,1,3}},
        {0,0,0,0,1,1,1,1}, {}, 3, false, 0.0, 0,0,0, 0,0,0, 0,0,1,
        lc::geo::Spline::RATIONAL, {1,0.5,1.5,1});
    const auto segments = spline.beziers();
    ASSERT_GT(segments.size(), 1u);
    bool found = false;
    for (const auto& segment : segments) {
        const auto p = segment->getCP().back();
        if (std::abs(p.x()-2.625) < 1e-12 && std::abs(p.y()+0.25) < 1e-12
            && std::abs(p.z()-1.6875) < 1e-12) found = true;
    }
    EXPECT_TRUE(found);
}

TEST(SplineBezierConversion, PeriodicQuadraticUsesSourceKnots) {
    const std::vector<lc::geo::Coordinate> controls{{0,0,1},{3,0,1},{3,2,1},{0,2,1},{0,0,1},{3,0,1}};
    const std::vector<double> knots{0,1,2,3,4,5,6,7,8};
    const lc::geo::Spline spline(controls, knots, {}, 2, true, 0.0,
        0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::PERIODIC);
    const auto segments = spline.beziers();
    ASSERT_GT(segments.size(), 1u);
    const auto first = segments.front()->getCP().front();
    const auto last = segments.back()->getCP().back();
    EXPECT_NEAR(first.x(), 1.5, 1e-10);
    EXPECT_NEAR(first.y(), 0.0, 1e-10);
    EXPECT_NEAR(first.z(), 1.0, 1e-10);
    EXPECT_NEAR(last.x(), 1.5, 1e-10);
    EXPECT_NEAR(last.y(), 0.0, 1e-10);
    EXPECT_NEAR(last.z(), 1.0, 1e-10);
    ASSERT_EQ(segments.size(), 4u);
    const std::vector<lc::geo::Coordinate> expected{{2.625,0.25,1},{2.625,1.75,1},
                                                    {0.375,1.75,1},{0.375,0.25,1}};
    for (std::size_t i = 0; i < segments.size(); ++i) {
        const auto mid = segments[i]->DirectValueAt(0.5);
        EXPECT_NEAR(mid.x(), expected[i].x(), 1e-10);
        EXPECT_NEAR(mid.y(), expected[i].y(), 1e-10);
        EXPECT_NEAR(mid.z(), expected[i].z(), 1e-10);
    }
}

TEST(SplineBezierConversion, PeriodicRationalMatchesIndependentPoints) {
    const std::vector<lc::geo::Coordinate> controls{{0,0,1},{3,0,1},{3,2,1},{0,2,1},{0,0,1},{3,0,1}};
    const std::vector<double> knots{0,1,2,3,4,5,6,7,8};
    const auto flags = static_cast<lc::geo::Spline::splineflag>(lc::geo::Spline::PERIODIC | lc::geo::Spline::RATIONAL);
    const lc::geo::Spline spline(controls, knots, {}, 2, true, 0.0,
        0,0,0, 0,0,0, 0,0,1, flags, {1,0.5,2,1.5,1,0.5});
    const auto segments = spline.beziers();
    ASSERT_GT(segments.size(), 4u);
    const auto first = segments.front()->getCP().front();
    const auto last = segments.back()->getCP().back();
    EXPECT_NEAR(first.x(), 1.0, 1e-10);
    EXPECT_NEAR(first.y(), 0.0, 1e-10);
    EXPECT_NEAR(last.x(), 1.0, 1e-10);
    EXPECT_NEAR(last.y(), 0.0, 1e-10);
    for (std::size_t i = 0; i < segments.size(); ++i) {
        const auto cp = segments[i]->getCP();
        for (const auto& p : cp) {
            EXPECT_GE(p.x(), -1e-10); EXPECT_LE(p.x(), 3.0 + 1e-10);
            EXPECT_GE(p.y(), -1e-10); EXPECT_LE(p.y(), 2.0 + 1e-10);
            EXPECT_NEAR(p.z(), 1.0, 1e-10);
        }
        if (i) {
            const auto previous = segments[i-1]->getCP().back();
            EXPECT_NEAR(previous.x(), cp.front().x(), 1e-10);
            EXPECT_NEAR(previous.y(), cp.front().y(), 1e-10);
            EXPECT_NEAR(previous.z(), cp.front().z(), 1e-10);
        }
    }
    const std::vector<lc::geo::Coordinate> expected{{2.5,2.0/3,1},
        {2.6785714285714284,1.9285714285714284,1},
        {0.5,1.8333333333333333,1},{0.1875,0.375,1}};
    for (const auto& point : expected) {
        double nearest = std::numeric_limits<double>::infinity();
        for (const auto& segment : segments) {
            const auto cp = segment->getCP();
            const auto d = cp.back() - cp.front(), q = point - cp.front();
            const double length2 = d.x()*d.x() + d.y()*d.y() + d.z()*d.z();
            const double t = length2 > 0 ? std::max(0.0, std::min(1.0,
                (q.x()*d.x() + q.y()*d.y() + q.z()*d.z()) / length2)) : 0;
            const auto e = point - (cp.front() + d*t);
            nearest = std::min(nearest, std::hypot(std::hypot(e.x(),e.y()),e.z()));
        }
        EXPECT_LE(nearest, 0.00031);
    }
}

TEST(SplineBezierConversion, NonClampedQuadraticUsesSourceDomain) {
    lc::geo::Spline spline({{0,0,0},{2,3,1},{4,0,2},{5,4,3},{8,1,4},{9,3,5}},
        {0,1,2,3,4,5,6,7,8}, {}, 2, false, 0.0, 0,0,0, 0,0,0, 0,0,1,
        static_cast<lc::geo::Spline::splineflag>(0));
    const auto segments = spline.beziers();
    ASSERT_EQ(segments.size(), 4u);
    const std::vector<lc::geo::Coordinate> expected{{2,2.25,1},{3.875,0.875,2},
        {5.25,3.125,3},{7.75,1.625,4}};
    const auto first = segments.front()->getCP().front();
    const auto last = segments.back()->getCP().back();
    EXPECT_NEAR(first.x(), 1, 1e-10); EXPECT_NEAR(first.y(), 1.5, 1e-10);
    EXPECT_NEAR(first.z(), 0.5, 1e-10);
    EXPECT_NEAR(last.x(), 8.5, 1e-10); EXPECT_NEAR(last.y(), 2, 1e-10);
    EXPECT_NEAR(last.z(), 4.5, 1e-10);
    for (std::size_t i=0; i<segments.size(); ++i) {
        const auto p=segments[i]->DirectValueAt(0.5);
        EXPECT_NEAR(p.x(),expected[i].x(),1e-10);
        EXPECT_NEAR(p.y(),expected[i].y(),1e-10);
        EXPECT_NEAR(p.z(),expected[i].z(),1e-10);
    }
    spline.populateCurve();
    EXPECT_EQ(spline.beziers().size(), segments.size());
}

TEST(SplineBezierConversion, NonClampedRationalUsesSourceDomain) {
    lc::geo::Spline spline({{0,0,0},{2,3,1},{4,0,2},{5,4,3},{8,1,4},{9,3,5}},
        {0,1,2,3,4,5,6,7,8}, {}, 2, false, 0.0, 0,0,0, 0,0,0, 0,0,1,
        lc::geo::Spline::RATIONAL, {1,0.5,2,1,3,1});
    const auto segments = spline.beziers();
    ASSERT_GT(segments.size(), 4u);
    const auto first=segments.front()->getCP().front(), last=segments.back()->getCP().back();
    EXPECT_NEAR(first.x(), 2.0/3, 1e-10); EXPECT_NEAR(first.y(), 1, 1e-10);
    EXPECT_NEAR(first.z(), 1.0/3, 1e-10);
    EXPECT_NEAR(last.x(), 8.25, 1e-10); EXPECT_NEAR(last.y(), 1.5, 1e-10);
    EXPECT_NEAR(last.z(), 4.25, 1e-10);
    const std::vector<lc::geo::Coordinate> expected{{2.333333333333333,1.5,1.1666666666666665},
        {4,0.4074074074074074,2.037037037037037},{5.636363636363636,2.454545454545454,3.090909090909091},
        {7.9,1.25,4}};
    for (const auto& point : expected) {
        double nearest=std::numeric_limits<double>::infinity();
        for (const auto& segment : segments) {
            const auto cp=segment->getCP();
            const auto d=cp.back()-cp.front(), q=point-cp.front();
            const double length2=d.x()*d.x()+d.y()*d.y()+d.z()*d.z();
            const double t=length2>0 ? std::max(0.0,std::min(1.0,
                (q.x()*d.x()+q.y()*d.y()+q.z()*d.z())/length2)) : 0;
            const auto e=point-(cp.front()+d*t);
            nearest=std::min(nearest,std::hypot(std::hypot(e.x(),e.y()),e.z()));
        }
        EXPECT_LE(nearest,0.00091);
    }
    spline.populateCurve();
    EXPECT_EQ(spline.beziers().size(), segments.size());
}

TEST(SplineBezierConversion, HighDegreeCurvesMatchIndependentPoints) {
    const std::vector<lc::geo::Coordinate> controls{{0,0,0},{1,3,1},{3,-1,2},{4,4,3},{7,-2,4},{9,1,5}};
    const auto check = [&](const lc::geo::Spline& spline, const std::vector<lc::geo::Coordinate>& expected) {
        const auto segments=spline.beziers();
        ASSERT_GT(segments.size(),2u);
        for (const auto& point : expected) {
            double nearest=std::numeric_limits<double>::infinity();
            for (const auto& segment : segments) {
                const auto cp=segment->getCP();
                const auto d=cp.back()-cp.front(), q=point-cp.front();
                const double length2=d.x()*d.x()+d.y()*d.y()+d.z()*d.z();
                const double t=length2>0 ? std::max(0.0,std::min(1.0,
                    (q.x()*d.x()+q.y()*d.y()+q.z()*d.z())/length2)) : 0;
                const auto e=point-(cp.front()+d*t);
                nearest=std::min(nearest,std::hypot(std::hypot(e.x(),e.y()),e.z()));
            }
            EXPECT_LE(nearest,0.00091);
        }
    };
    const lc::geo::Spline fourth(controls, {0,0,0,0,0,0.3,1,1,1,1,1}, {},
        4, false, 0, 0,0,0, 0,0,0, 0,0,1, static_cast<lc::geo::Spline::splineflag>(0));
    check(fourth, {{0,0,0},{1.5406875,1.858125,1.25625},{2.611,1.29,1.9},
                   {5.2581875,0.720625,3.31875},{9,1,5}});
    const lc::geo::Spline fifth(controls, {0,0,0,0,0,0,1,1,1,1,1,1}, {},
        5, false, 0, 0,0,0, 0,0,0, 0,0,1, lc::geo::Spline::RATIONAL, {1,0.5,2,1,3,1});
    check(fifth, {{0,0,0},{1.233959199975574,0.41786496761464037,0.8955218338142932},
                  {2.6817075485626303,0.23989623099150023,1.8257650855250669},
                  {5.647547254537985,-0.3939091276257335,3.416716987602413},{9,1,5}});
}

TEST(SplineBezierConversion, HighDegreeInvalidInputHasNoInventedGeometry) {
    const lc::geo::Spline invalid({{0,0},{1,1},{2,0},{3,1},{4,0},{5,1}},
        {0,0,0,0,0,0,1,1,1,1,1,1}, {}, 5, false, 0, 0,0,0, 0,0,0, 0,0,1,
        lc::geo::Spline::RATIONAL, {1,1,0,1,1,1});
    EXPECT_TRUE(invalid.beziers().empty());
    const std::vector<lc::geo::Coordinate> many(66,{0,0,0});
    std::vector<double> knots(132,1);
    std::fill(knots.begin(),knots.begin()+66,0);
    const lc::geo::Spline overLimit(many,knots,{},65,false,0,0,0,0,0,0,0,0,0,1,
        static_cast<lc::geo::Spline::splineflag>(0));
    EXPECT_TRUE(overLimit.beziers().empty());
}
