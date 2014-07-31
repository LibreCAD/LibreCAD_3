#include "code.h"
#include "gtest/gtest.h"
#include "entitytest.h"
using namespace std;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


TEST(test, quad) {
    test c;

    EXPECT_DOUBLE_EQ(-2, c.quad(5, 6)[0]);
    EXPECT_DOUBLE_EQ(-3, c.quad(5, 6)[1]);
}

TEST(test, cubic) {
    test c;

    EXPECT_DOUBLE_EQ(-0.19806226419516171, c.cubic(5, 6, 1)[0]);
    EXPECT_DOUBLE_EQ(-1.5549581320873713, c.cubic(5, 6, 1)[1]);
    EXPECT_DOUBLE_EQ(-3.2469796037174667, c.cubic(5, 6, 1)[2]);
}

TEST(test, quartic) {
    test c;

    EXPECT_DOUBLE_EQ(-2.0, c.quartic(5, 6, 1, 2)[0]);
    EXPECT_DOUBLE_EQ(-3.1038034027355366, c.quartic(5, 6, 1, 2)[1]);
}

TEST(test, testin) {
    test a;
    const double rootY = 200. / sqrt(100.*100. + 1.);
    const double rootX = 100.*rootY;
    EXPECT_DOUBLE_EQ(-rootX, a.testin()[0].x());
    EXPECT_DOUBLE_EQ(-rootY, a.testin()[0].y());
    EXPECT_DOUBLE_EQ(rootX, a.testin()[1].x());
    EXPECT_DOUBLE_EQ(rootY, a.testin()[1].y());
}

TEST(entitytest, LineMove) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, round(a.LineMove(_a)[0]->start().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.LineMove(_a)[0]->start().y()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineMove(_a)[0]->end().x()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineMove(_a)[0]->end().y()));

    EXPECT_DOUBLE_EQ(10.0, round(a.LineMove(_a)[0]->start().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.LineMove(_a)[0]->start().y()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineMove(_a)[0]->end().x()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineMove(_a)[0]->end().y()));

    EXPECT_DOUBLE_EQ(10.0, round(a.LineMove(_a)[0]->start().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.LineMove(_a)[0]->start().y()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineMove(_a)[0]->end().x()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineMove(_a)[0]->end().y()));

    EXPECT_DOUBLE_EQ(10.0, round(a.LineMove(_a)[0]->start().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.LineMove(_a)[0]->start().y()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineMove(_a)[0]->end().x()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineMove(_a)[0]->end().y()));
}

TEST(entitytest, LineCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, round(a.LineCopy(_a)[0]->start().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.LineCopy(_a)[0]->start().y()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineCopy(_a)[0]->end().x()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineCopy(_a)[0]->end().y()));

    EXPECT_DOUBLE_EQ(10.0, round(a.LineCopy(_a)[0]->start().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.LineCopy(_a)[0]->start().y()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineCopy(_a)[0]->end().x()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineCopy(_a)[0]->end().y()));

    EXPECT_DOUBLE_EQ(10.0, round(a.LineCopy(_a)[0]->start().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.LineCopy(_a)[0]->start().y()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineCopy(_a)[0]->end().x()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineCopy(_a)[0]->end().y()));

    EXPECT_DOUBLE_EQ(10.0, round(a.LineCopy(_a)[0]->start().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.LineCopy(_a)[0]->start().y()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineCopy(_a)[0]->end().x()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineCopy(_a)[0]->end().y()));
}

TEST(entitytest, LineRotate) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate(_a, angle)[0]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate(_a, angle)[0]->start().y()));
    EXPECT_DOUBLE_EQ(-100.0, round(a.LineRotate(_a, angle)[0]->end().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.LineRotate(_a, angle)[0]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate(_a, angle)[0]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate(_a, angle)[0]->start().y()));
    EXPECT_DOUBLE_EQ(-100.0, round(a.LineRotate(_a, angle)[0]->end().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.LineRotate(_a, angle)[0]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate(_a, angle)[0]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate(_a, angle)[0]->start().y()));
    EXPECT_DOUBLE_EQ(-100.0, round(a.LineRotate(_a, angle)[0]->end().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.LineRotate(_a, angle)[0]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate(_a, angle)[0]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate(_a, angle)[0]->start().y()));
    EXPECT_DOUBLE_EQ(-100.0, round(a.LineRotate(_a, angle)[0]->end().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.LineRotate(_a, angle)[0]->end().y()));
}

TEST(entitytest, LineScale) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2,2);
    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale(_a, factor)[0]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale(_a, factor)[0]->start().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale(_a, factor)[0]->end().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale(_a, factor)[0]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale(_a, factor)[0]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale(_a, factor)[0]->start().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale(_a, factor)[0]->end().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale(_a, factor)[0]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale(_a, factor)[0]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale(_a, factor)[0]->start().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale(_a, factor)[0]->end().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale(_a, factor)[0]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale(_a, factor)[0]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale(_a, factor)[0]->start().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale(_a, factor)[0]->end().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale(_a, factor)[0]->end().y()));
}


TEST(entitytest, CircleMove) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleMove(_a)[0]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleMove(_a)[0]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleMove(_a)[0]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleMove(_a)[0]->radius()));
}

TEST(entitytest, CircleCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleCopy(_a)[0]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleCopy(_a)[0]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleCopy(_a)[0]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleCopy(_a)[0]->radius()));
}


TEST(entitytest, CircleRotate) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate(_a, angle)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleRotate(_a, angle)[0]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate(_a, angle)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleRotate(_a, angle)[0]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate(_a, angle)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleRotate(_a, angle)[0]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate(_a, angle)[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleRotate(_a, angle)[0]->radius()));
}


TEST(entitytest, CircleScale) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2.,2.);
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.CircleScale(_a, factor)[0]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.CircleScale(_a, factor)[0]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.CircleScale(_a, factor)[0]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.CircleScale(_a, factor)[0]->radius()));
}


TEST(entitytest, ArcMove) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcMove(_a)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcMove(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcMove(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcMove(_a)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcMove(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcMove(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcMove(_a)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcMove(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcMove(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcMove(_a)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcMove(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcMove(_a)[0]->endAngle()));
}

TEST(entitytest, ArcCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcCopy(_a)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcCopy(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcCopy(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcCopy(_a)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcCopy(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcCopy(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcCopy(_a)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcCopy(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcCopy(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcCopy(_a)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcCopy(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcCopy(_a)[0]->endAngle()));
}

TEST(entitytest, ArcRotate) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate(_a, angle)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcRotate(_a, angle)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa+angle), round(a.ArcRotate(_a, angle)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea+angle), round(a.ArcRotate(_a, angle)[0]->endAngle()));

    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate(_a, angle)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcRotate(_a, angle)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa+angle), round(a.ArcRotate(_a, angle)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea+angle), round(a.ArcRotate(_a, angle)[0]->endAngle()));

    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate(_a, angle)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcRotate(_a, angle)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa+angle), round(a.ArcRotate(_a, angle)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea+angle), round(a.ArcRotate(_a, angle)[0]->endAngle()));

    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate(_a, angle)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcRotate(_a, angle)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa+angle), round(a.ArcRotate(_a, angle)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea+angle), round(a.ArcRotate(_a, angle)[0]->endAngle()));
}

TEST(entitytest, ArcScale) {
    entitytest a;
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2,2);
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.ArcScale(_a, factor)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcScale(_a, factor)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcScale(_a, factor)[0]->endAngle()));

    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2,2);
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.ArcScale(_a, factor)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcScale(_a, factor)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcScale(_a, factor)[0]->endAngle()));

    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2,2);
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.ArcScale(_a, factor)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcScale(_a, factor)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcScale(_a, factor)[0]->endAngle()));

    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2,2);
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.ArcScale(_a, factor)[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcScale(_a, factor)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcScale(_a, factor)[0]->endAngle()));
}


TEST(entitytest, EllipseMove) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseMove(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseMove(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseMove(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseMove(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseMove(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseMove(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove(_a)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseMove(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseMove(_a)[0]->endAngle()));
}

TEST(entitytest, EllipseCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseCopy(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseCopy(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseCopy(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseCopy(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseCopy(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseCopy(_a)[0]->endAngle()));

    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy(_a)[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy(_a)[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy(_a)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseCopy(_a)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseCopy(_a)[0]->endAngle()));
}

TEST(entitytest, EllipseRotate) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate(_a, angle)[0]->center().y()));
    lc::geo::Coordinate rotatedMajorP=lc::geo::Coordinate(100., 100.).rotate(_a, angle);
    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.x())/1e4, round(a.EllipseRotate(_a, angle)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.y())/1e4, round(a.EllipseRotate(_a, angle)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseRotate(_a, angle)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseRotate(_a, angle)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseRotate(_a, angle)[0]->endAngle()));

    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate(_a, angle)[0]->center().y()));
    lc::geo::Coordinate rotatedMajorP=lc::geo::Coordinate(100., 100.).rotate(_a, angle);
    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.x())/1e4, round(a.EllipseRotate(_a, angle)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.y())/1e4, round(a.EllipseRotate(_a, angle)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseRotate(_a, angle)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseRotate(_a, angle)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseRotate(_a, angle)[0]->endAngle()));

    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate(_a, angle)[0]->center().y()));
    lc::geo::Coordinate rotatedMajorP=lc::geo::Coordinate(100., 100.).rotate(_a, angle);
    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.x())/1e4, round(a.EllipseRotate(_a, angle)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.y())/1e4, round(a.EllipseRotate(_a, angle)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseRotate(_a, angle)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseRotate(_a, angle)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseRotate(_a, angle)[0]->endAngle()));

    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate(_a, angle)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate(_a, angle)[0]->center().y()));
    lc::geo::Coordinate rotatedMajorP=lc::geo::Coordinate(100., 100.).rotate(_a, angle);
    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.x())/1e4, round(a.EllipseRotate(_a, angle)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.y())/1e4, round(a.EllipseRotate(_a, angle)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseRotate(_a, angle)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseRotate(_a, angle)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseRotate(_a, angle)[0]->endAngle()));
}

TEST(entitytest, EllipseScale) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2,2);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseScale(_a, factor)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseScale(_a, factor)[0]->endAngle()));

    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2,2);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseScale(_a, factor)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseScale(_a, factor)[0]->endAngle()));

    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2,2);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseScale(_a, factor)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseScale(_a, factor)[0]->endAngle()));

    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2,2);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale(_a, factor)[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale(_a, factor)[0]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale(_a, factor)[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseScale(_a, factor)[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseScale(_a, factor)[0]->endAngle()));
}
