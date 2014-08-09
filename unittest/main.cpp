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
    std::vector <Line_CSPtr>_a = a.LineMove();

    EXPECT_DOUBLE_EQ(10.0, round(_a[0]->start().x()));
    EXPECT_DOUBLE_EQ(10.0, round(_a[0]->start().y()));
    EXPECT_DOUBLE_EQ(110.0, round(_a[0]->end().x()));
    EXPECT_DOUBLE_EQ(110.0, round(_a[0]->end().y()));

    EXPECT_DOUBLE_EQ(130.0, round(a.LineMove()[1]->start().x()));
    EXPECT_DOUBLE_EQ(20.0, round(a.LineMove()[1]->start().y()));
    EXPECT_DOUBLE_EQ(120.0, round(a.LineMove()[1]->end().x()));
    EXPECT_DOUBLE_EQ(130.0, round(a.LineMove()[1]->end().y()));

    EXPECT_DOUBLE_EQ(530.0, round(a.LineMove()[2]->start().x()));
    EXPECT_DOUBLE_EQ(80.0, round(a.LineMove()[2]->start().y()));
    EXPECT_DOUBLE_EQ(70.0, round(a.LineMove()[2]->end().x()));
    EXPECT_DOUBLE_EQ(-80.0, round(a.LineMove()[2]->end().y()));

    EXPECT_DOUBLE_EQ(180.0, round(a.LineMove()[3]->start().x()));
    EXPECT_DOUBLE_EQ(730.0, round(a.LineMove()[3]->start().y()));
    EXPECT_DOUBLE_EQ(-120.0, round(a.LineMove()[3]->end().x()));
    EXPECT_DOUBLE_EQ(80.0, round(a.LineMove()[3]->end().y()));
}

TEST(entitytest, LineCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, round(a.LineCopy()[1]->start().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.LineCopy()[1]->start().y()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineCopy()[1]->end().x()));
    EXPECT_DOUBLE_EQ(110.0, round(a.LineCopy()[1]->end().y()));

    EXPECT_DOUBLE_EQ(130.0, round(a.LineCopy()[3]->start().x()));
    EXPECT_DOUBLE_EQ(20.0, round(a.LineCopy()[3]->start().y()));
    EXPECT_DOUBLE_EQ(120.0, round(a.LineCopy()[3]->end().x()));
    EXPECT_DOUBLE_EQ(130.0, round(a.LineCopy()[3]->end().y()));

    EXPECT_DOUBLE_EQ(530.0, round(a.LineCopy()[5]->start().x()));
    EXPECT_DOUBLE_EQ(80.0, round(a.LineCopy()[5]->start().y()));
    EXPECT_DOUBLE_EQ(70.0, round(a.LineCopy()[5]->end().x()));
    EXPECT_DOUBLE_EQ(-80.0, round(a.LineCopy()[5]->end().y()));

    EXPECT_DOUBLE_EQ(180.0, round(a.LineCopy()[7]->start().x()));
    EXPECT_DOUBLE_EQ(730.0, round(a.LineCopy()[7]->start().y()));
    EXPECT_DOUBLE_EQ(-120.0, round(a.LineCopy()[7]->end().x()));
    EXPECT_DOUBLE_EQ(80.0, round(a.LineCopy()[7]->end().y()));
}

TEST(entitytest, LineRotate) {
    entitytest a;
    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate()[0]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate()[0]->start().y()));
    EXPECT_DOUBLE_EQ(-100.0, round(a.LineRotate()[0]->end().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.LineRotate()[0]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate()[1]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate()[1]->start().y()));
    EXPECT_DOUBLE_EQ(-100.0, round(a.LineRotate()[1]->end().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.LineRotate()[1]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate()[2]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate()[2]->start().y()));
    EXPECT_DOUBLE_EQ(-100.0, round(a.LineRotate()[2]->end().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.LineRotate()[2]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate()[3]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineRotate()[3]->start().y()));
    EXPECT_DOUBLE_EQ(-100.0, round(a.LineRotate()[3]->end().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.LineRotate()[3]->end().y()));
}

TEST(entitytest, LineScale) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2,2);
    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale()[0]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale()[0]->start().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale()[0]->end().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale()[0]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale()[1]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale()[1]->start().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale()[1]->end().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale()[1]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale()[2]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale()[2]->start().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale()[2]->end().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale()[2]->end().y()));

    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale()[3]->start().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.LineScale()[3]->start().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale()[3]->end().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.LineScale()[3]->end().y()));
}


TEST(entitytest, CircleMove) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove()[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove()[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleMove()[0]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove()[1]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove()[1]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleMove()[1]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove()[2]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove()[2]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleMove()[2]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove()[3]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleMove()[3]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleMove()[3]->radius()));
}

TEST(entitytest, CircleCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy()[1]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy()[1]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleCopy()[1]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy()[3]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy()[3]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleCopy()[3]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy()[5]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy()[5]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleCopy()[5]->radius()));

    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy()[7]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.CircleCopy()[7]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleCopy()[7]->radius()));
}


TEST(entitytest, CircleRotate) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate()[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate()[0]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleRotate()[0]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate()[1]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate()[1]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleRotate()[1]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate()[2]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate()[2]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleRotate()[2]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate()[3]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleRotate()[3]->center().y()));
    EXPECT_DOUBLE_EQ(50.0, round(a.CircleRotate()[3]->radius()));
}


TEST(entitytest, CircleScale) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factor(2.,2.);
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale()[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale()[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.CircleScale()[0]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale()[1]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale()[1]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.CircleScale()[1]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale()[2]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale()[2]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.CircleScale()[2]->radius()));

    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale()[3]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.CircleScale()[3]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.CircleScale()[3]->radius()));
}


TEST(entitytest, ArcMove) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove()[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove()[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcMove()[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcMove()[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcMove()[0]->endAngle()));

    lc::geo::Coordinate _b(10.0,10.0);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove()[1]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove()[1]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcMove()[1]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcMove()[1]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcMove()[1]->endAngle()));

    lc::geo::Coordinate _c(10.0,10.0);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove()[2]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove()[2]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcMove()[2]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcMove()[2]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcMove()[2]->endAngle()));

    lc::geo::Coordinate _d(10.0,10.0);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove()[3]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcMove()[3]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcMove()[3]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcMove()[3]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcMove()[3]->endAngle()));
}

TEST(entitytest, ArcCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy()[1]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy()[1]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcCopy()[1]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcCopy()[1]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcCopy()[1]->endAngle()));

    lc::geo::Coordinate _b(10.0,10.0);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy()[3]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy()[3]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcCopy()[3]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcCopy()[3]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcCopy()[3]->endAngle()));

    lc::geo::Coordinate _c(10.0,10.0);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy()[5]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy()[5]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcCopy()[5]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcCopy()[5]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcCopy()[5]->endAngle()));

    lc::geo::Coordinate _d(10.0,10.0);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy()[7]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.ArcCopy()[7]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.ArcCopy()[7]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcCopy()[7]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcCopy()[7]->endAngle()));
}

//TEST(entitytest, ArcRotate) {
//    entitytest a;
//    lc::geo::Coordinate _a(0.0,0.0);
//    auto angle = 90 * 0.0174532925;
//    auto sa = 30 * 0.0174532925;
//    auto ea = 120 * 0.0174532925;
//    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate()[0]->center().x()));
//    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate()[0]->center().y()));
//    EXPECT_DOUBLE_EQ(100.0, round(a.ArcRotate()[0]->radius()));
//    EXPECT_DOUBLE_EQ(round(sa+angle), round(a.ArcRotate()[0]->startAngle()));
//    EXPECT_DOUBLE_EQ(round(ea+angle), round(a.ArcRotate()[0]->endAngle()));

//    lc::geo::Coordinate _b(0.0,0.0);
//    angle = 90 * 0.0174532925;
//    sa = 30 * 0.0174532925;
//    ea = 120 * 0.0174532925;
//    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate()[1]->center().x()));
//    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate()[1]->center().y()));
//    EXPECT_DOUBLE_EQ(100.0, round(a.ArcRotate()[1]->radius()));
//    EXPECT_DOUBLE_EQ(round(sa+angle), round(a.ArcRotate()[1]->startAngle()));
//    EXPECT_DOUBLE_EQ(round(ea+angle), round(a.ArcRotate()[1]->endAngle()));

//    lc::geo::Coordinate _c(0.0,0.0);
//    angle = 90 * 0.0174532925;
//    sa = 30 * 0.0174532925;
//    ea = 120 * 0.0174532925;
//    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate()[2]->center().x()));
//    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate()[2]->center().y()));
//    EXPECT_DOUBLE_EQ(100.0, round(a.ArcRotate()[2]->radius()));
//    EXPECT_DOUBLE_EQ(round(sa+angle), round(a.ArcRotate()[2]->startAngle()));
//    EXPECT_DOUBLE_EQ(round(ea+angle), round(a.ArcRotate()[2]->endAngle()));

//    lc::geo::Coordinate _d(0.0,0.0);
//    angle = 90 * 0.0174532925;
//    sa = 30 * 0.0174532925;
//    ea = 120 * 0.0174532925;
//    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate()[3]->center().x()));
//    EXPECT_DOUBLE_EQ(0.0, round(a.ArcRotate()[3]->center().y()));
//    EXPECT_DOUBLE_EQ(100.0, round(a.ArcRotate()[3]->radius()));
//    EXPECT_DOUBLE_EQ(round(sa+angle), round(a.ArcRotate()[3]->startAngle()));
//    EXPECT_DOUBLE_EQ(round(ea+angle), round(a.ArcRotate()[3]->endAngle()));
//}

TEST(entitytest, ArcScale) {
    entitytest a;
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factorA(2,2);
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale()[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale()[0]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.ArcScale()[0]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcScale()[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcScale()[0]->endAngle()));

    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    lc::geo::Coordinate _b(0.0,0.0);
    lc::geo::Coordinate factorB(2,2);
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale()[1]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale()[1]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.ArcScale()[1]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcScale()[1]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcScale()[1]->endAngle()));

    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    lc::geo::Coordinate _c(0.0,0.0);
    lc::geo::Coordinate factorC(2,2);
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale()[2]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale()[2]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.ArcScale()[2]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcScale()[2]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcScale()[2]->endAngle()));

    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    lc::geo::Coordinate _d(0.0,0.0);
    lc::geo::Coordinate factorD(2,2);
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale()[3]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.ArcScale()[3]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.ArcScale()[3]->radius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.ArcScale()[3]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.ArcScale()[3]->endAngle()));
}


TEST(entitytest, EllipseMove) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove()[0]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove()[0]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseMove()[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseMove()[0]->endAngle()));

    lc::geo::Coordinate _b(10.0,10.0);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove()[1]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove()[1]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[1]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[1]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[1]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseMove()[1]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseMove()[1]->endAngle()));

    lc::geo::Coordinate _c(10.0,10.0);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove()[2]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove()[2]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[2]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[2]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[2]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseMove()[2]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseMove()[2]->endAngle()));

    lc::geo::Coordinate _d(10.0,10.0);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove()[3]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseMove()[3]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[3]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[3]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseMove()[3]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseMove()[3]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseMove()[3]->endAngle()));
}

TEST(entitytest, EllipseCopy) {
    entitytest a;

    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy()[1]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy()[1]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[1]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[1]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[1]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseCopy()[1]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseCopy()[1]->endAngle()));

    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy()[3]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy()[3]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[3]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[3]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[3]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseCopy()[3]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseCopy()[3]->endAngle()));

    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy()[5]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy()[5]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[5]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[5]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[5]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseCopy()[5]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseCopy()[5]->endAngle()));

    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy()[7]->center().x()));
    EXPECT_DOUBLE_EQ(10.0, round(a.EllipseCopy()[7]->center().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[7]->majorP().x()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[7]->majorP().y()));
    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseCopy()[7]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseCopy()[7]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseCopy()[7]->endAngle()));
}

//TEST(entitytest, EllipseRotate) {
//    entitytest a;
//    lc::geo::Coordinate _a(0.0,0.0);
//    auto angle = 90 * 0.0174532925;
//    auto sa = 30 * 0.0174532925;
//    auto ea = 120 * 0.0174532925;
//    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate()[0]->center().x()));
//    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate()[0]->center().y()));
//    lc::geo::Coordinate rotatedMajorP=lc::geo::Coordinate(100., 100.).rotate();
//    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.x())/1e4, round(a.EllipseRotate()[0]->majorP().x()));
//    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.y())/1e4, round(a.EllipseRotate()[0]->majorP().y()));
//    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseRotate()[0]->minorRadius()));
//    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseRotate()[0]->startAngle()));
//    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseRotate()[0]->endAngle()));

//    lc::geo::Coordinate _b(0.0,0.0);
//    angle = 90 * 0.0174532925;
//    sa = 30 * 0.0174532925;
//    ea = 120 * 0.0174532925;
//    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate()[1]->center().x()));
//    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate()[1]->center().y()));
//    lc::geo::Coordinate rotatedMajorP=lc::geo::Coordinate(100., 100.).rotate();
//    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.x())/1e4, round(a.EllipseRotate()[1]->majorP().x()));
//    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.y())/1e4, round(a.EllipseRotate()[1]->majorP().y()));
//    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseRotate()[1]->minorRadius()));
//    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseRotate()[1]->startAngle()));
//    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseRotate()[1]->endAngle()));

//    lc::geo::Coordinate _c(0.0,0.0);
//    angle = 90 * 0.0174532925;
//    sa = 30 * 0.0174532925;
//    ea = 120 * 0.0174532925;
//    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate()[2]->center().x()));
//    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate()[2]->center().y()));
//    lc::geo::Coordinate rotatedMajorP=lc::geo::Coordinate(100., 100.).rotate();
//    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.x())/1e4, round(a.EllipseRotate()[2]->majorP().x()));
//    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.y())/1e4, round(a.EllipseRotate()[2]->majorP().y()));
//    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseRotate()[2]->minorRadius()));
//    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseRotate()[2]->startAngle()));
//    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseRotate()[2]->endAngle()));

//    lc::geo::Coordinate _d(0.0,0.0);
//    angle = 90 * 0.0174532925;
//    sa = 30 * 0.0174532925;
//    ea = 120 * 0.0174532925;
//    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate()[3]->center().x()));
//    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseRotate()[3]->center().y()));
//    lc::geo::Coordinate rotatedMajorP=lc::geo::Coordinate(100., 100.).rotate();
//    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.x())/1e4, round(a.EllipseRotate()[3]->majorP().x()));
//    EXPECT_DOUBLE_EQ(round(1e4*rotatedMajorP.y())/1e4, round(a.EllipseRotate()[3]->majorP().y()));
//    EXPECT_DOUBLE_EQ(100.0, round(a.EllipseRotate()[3]->minorRadius()));
//    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseRotate()[3]->startAngle()));
//    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseRotate()[3]->endAngle()));
//}

TEST(entitytest, EllipseScale) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    lc::geo::Coordinate factorA(2,2);
    auto sa = 30 * 0.0174532925;
    auto ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale()[0]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale()[0]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[0]->majorP().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[0]->majorP().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[0]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseScale()[0]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseScale()[0]->endAngle()));

    lc::geo::Coordinate _b(0.0,0.0);
    lc::geo::Coordinate factorB(2,2);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale()[1]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale()[1]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[1]->majorP().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[1]->majorP().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[1]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseScale()[1]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseScale()[1]->endAngle()));

    lc::geo::Coordinate _c(0.0,0.0);
    lc::geo::Coordinate factorC(2,2);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale()[2]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale()[2]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[2]->majorP().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[2]->majorP().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[2]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseScale()[2]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseScale()[2]->endAngle()));

    lc::geo::Coordinate _d(0.0,0.0);
    lc::geo::Coordinate factorD(2,2);
    sa = 30 * 0.0174532925;
    ea = 120 * 0.0174532925;
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale()[3]->center().x()));
    EXPECT_DOUBLE_EQ(0.0, round(a.EllipseScale()[3]->center().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[3]->majorP().x()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[3]->majorP().y()));
    EXPECT_DOUBLE_EQ(200.0, round(a.EllipseScale()[3]->minorRadius()));
    EXPECT_DOUBLE_EQ(round(sa), round(a.EllipseScale()[3]->startAngle()));
    EXPECT_DOUBLE_EQ(round(ea), round(a.EllipseScale()[3]->endAngle()));
}
