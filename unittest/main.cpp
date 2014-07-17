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
    EXPECT_DOUBLE_EQ(10.0, a.LineMove(_a)->start().x());
    EXPECT_DOUBLE_EQ(10.0, a.LineMove(_a)->start().y());
    EXPECT_DOUBLE_EQ(110.0, a.LineMove(_a)->end().x());
    EXPECT_DOUBLE_EQ(110.0, a.LineMove(_a)->end().y());
}

TEST(entitytest, LineCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, a.LineCopy(_a)->start().x());
    EXPECT_DOUBLE_EQ(10.0, a.LineCopy(_a)->start().y());
    EXPECT_DOUBLE_EQ(110.0, a.LineCopy(_a)->end().x());
    EXPECT_DOUBLE_EQ(110.0, a.LineCopy(_a)->end().y());
}

TEST(entitytest, LineRotate) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, a.LineRotate(_a, angle)->start().x());
    EXPECT_DOUBLE_EQ(10.0, a.LineRotate(_a, angle)->start().y());
    EXPECT_DOUBLE_EQ(110.0, a.LineRotate(_a, angle)->end().x());
    EXPECT_DOUBLE_EQ(110.0, a.LineRotate(_a, angle)->end().y());
}

TEST(entitytest, LineScale) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    lc::geo::Coordinate factor(2,-2);
    EXPECT_DOUBLE_EQ(10.0, a.LineScale(_a, factor)->start().x());
    EXPECT_DOUBLE_EQ(10.0, a.LineScale(_a, factor)->start().y());
    EXPECT_DOUBLE_EQ(110.0, a.LineScale(_a, factor)->end().x());
    EXPECT_DOUBLE_EQ(110.0, a.LineScale(_a, factor)->end().y());
}


TEST(entitytest, CircleMove) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, a.CircleMove(_a)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.CircleMove(_a)->center().y());
    EXPECT_DOUBLE_EQ(110.0, a.CircleMove(_a)->radius());
}

TEST(entitytest, CircleCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, a.CircleCopy(_a)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.CircleCopy(_a)->center().y());
    EXPECT_DOUBLE_EQ(110.0, a.CircleCopy(_a)->radius());
}


TEST(entitytest, CircleRotate) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, a.CircleRotate(_a, angle)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.CircleRotate(_a, angle)->center().y());
    EXPECT_DOUBLE_EQ(110.0, a.CircleRotate(_a, angle)->radius());
}


TEST(entitytest, CircleScale) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    lc::geo::Coordinate factor(2.,-2.);
    EXPECT_DOUBLE_EQ(10.0, a.CircleScale(_a, factor)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.CircleScale(_a, factor)->center().y());
    EXPECT_DOUBLE_EQ(110.0, a.CircleScale(_a, factor)->radius());
}


TEST(entitytest, ArcMove) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, a.ArcMove(_a)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.ArcMove(_a)->center().y());
    EXPECT_DOUBLE_EQ(110.0, a.ArcMove(_a)->radius());
    EXPECT_DOUBLE_EQ(110.0, a.ArcMove(_a)->startAngle());
    EXPECT_DOUBLE_EQ(110.0, a.ArcMove(_a)->endAngle());

}

TEST(entitytest, ArcCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, a.ArcCopy(_a)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.ArcCopy(_a)->center().y());
    EXPECT_DOUBLE_EQ(110.0, a.ArcCopy(_a)->radius());
    EXPECT_DOUBLE_EQ(110.0, a.ArcCopy(_a)->startAngle());
    EXPECT_DOUBLE_EQ(110.0, a.ArcCopy(_a)->endAngle());
}

TEST(entitytest, ArcRotate) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, a.ArcRotate(_a, angle)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.ArcRotate(_a, angle)->center().y());
    EXPECT_DOUBLE_EQ(110.0, a.ArcRotate(_a, angle)->radius());
    EXPECT_DOUBLE_EQ(110.0, a.ArcRotate(_a, angle)->startAngle());
    EXPECT_DOUBLE_EQ(110.0, a.ArcRotate(_a, angle)->endAngle());
}

TEST(entitytest, ArcScale) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    lc::geo::Coordinate factor(2,-2);
    EXPECT_DOUBLE_EQ(10.0, a.ArcScale(_a, factor)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.ArcScale(_a, factor)->center().y());
    EXPECT_DOUBLE_EQ(110.0, a.ArcScale(_a, factor)->radius());
    EXPECT_DOUBLE_EQ(110.0, a.ArcScale(_a, factor)->startAngle());
    EXPECT_DOUBLE_EQ(110.0, a.ArcScale(_a, factor)->endAngle());
}

TEST(entitytest, EllipseMove) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, a.EllipseMove(_a)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseMove(_a)->center().y());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseMove(_a)->majorP().x());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseMove(_a)->majorP().y());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseMove(_a)->minorRadius());
    EXPECT_DOUBLE_EQ(110.0, a.EllipseMove(_a)->startAngle());
    EXPECT_DOUBLE_EQ(110.0, a.EllipseMove(_a)->endAngle());
}

TEST(entitytest, EllipseCopy) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    EXPECT_DOUBLE_EQ(10.0, a.EllipseCopy(_a)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseCopy(_a)->center().y());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseCopy(_a)->majorP().x());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseCopy(_a)->majorP().y());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseCopy(_a)->minorRadius());
    EXPECT_DOUBLE_EQ(110.0, a.EllipseCopy(_a)->startAngle());
    EXPECT_DOUBLE_EQ(110.0, a.EllipseCopy(_a)->endAngle());
}

TEST(entitytest, EllipseRotate) {
    entitytest a;
    lc::geo::Coordinate _a(0.0,0.0);
    auto angle = 90 * 0.0174532925;
    EXPECT_DOUBLE_EQ(10.0, a.EllipseRotate(_a, angle)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseRotate(_a, angle)->center().y());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseRotate(_a, angle)->majorP().x());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseRotate(_a, angle)->majorP().y());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseRotate(_a, angle)->minorRadius());
    EXPECT_DOUBLE_EQ(110.0, a.EllipseRotate(_a, angle)->startAngle());
    EXPECT_DOUBLE_EQ(110.0, a.EllipseRotate(_a, angle)->endAngle());
}

TEST(entitytest, EllipseScale) {
    entitytest a;
    lc::geo::Coordinate _a(10.0,10.0);
    lc::geo::Coordinate factor(2,-2);
    EXPECT_DOUBLE_EQ(10.0, a.EllipseScale(_a, factor)->center().x());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseScale(_a, factor)->center().y());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseScale(_a, factor)->majorP().x());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseScale(_a, factor)->majorP().y());
    EXPECT_DOUBLE_EQ(10.0, a.EllipseScale(_a, factor)->minorRadius());
    EXPECT_DOUBLE_EQ(110.0, a.EllipseScale(_a, factor)->startAngle());
    EXPECT_DOUBLE_EQ(110.0, a.EllipseScale(_a, factor)->endAngle());
}
