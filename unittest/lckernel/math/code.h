#pragma once

#include <gtest/gtest.h>
#include <vector>
#include <cad/primitive/circle.h>
#include <cad/primitive/line.h>
#include <cad/meta/layer.h>
#include <cad/primitive/point.h>

class test {
    public:
        test() {
        }

        std::vector<double> quad(double x, double c);
        std::vector<double> cubic(double x2, double x, double c);
        std::vector<double> quartic(double x3, double x2, double x, double c);
        std::vector<lc::geo::Coordinate> intr(lc::entity::Line&, lc::entity::Circle&);
        std::vector<lc::geo::Coordinate> testin();
        template <typename e1, typename e2>
        std::vector<lc::geo::Coordinate>intersec(e1, e2);
};

TEST(test, quad) {
    test c;

    EXPECT_DOUBLE_EQ(-2, c.quad(5, 6)[0]);
    EXPECT_DOUBLE_EQ(-3, c.quad(5, 6)[1]);

    EXPECT_TRUE(fabs(c.quad(100, 50)[0]) - 0.50252531694167146 < 0.000000001);
    EXPECT_DOUBLE_EQ(-99.497474683058329, c.quad(100, 50)[1]);

    //    EXPECT_DOUBLE_EQ(-2, c.quad(5, 6)[0]);
    //    EXPECT_DOUBLE_EQ(-3, c.quad(5, 6)[1]);

    //    EXPECT_DOUBLE_EQ(-2, c.quad(5, 6)[0]);
    //    EXPECT_DOUBLE_EQ(-3, c.quad(5, 6)[1]);
}

TEST(test, cubic) {
    test c;

    EXPECT_TRUE(fabs(c.cubic(5, 6, 1)[0]) - 0.19806226419516171 < .00000001);
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
    const double rootY = 200. / sqrt(100. * 100. + 1.);
    const double rootX = 100. * rootY;
    EXPECT_DOUBLE_EQ(-rootX, a.testin()[0].x());
    EXPECT_DOUBLE_EQ(-rootY, a.testin()[0].y());
    EXPECT_DOUBLE_EQ(rootX, a.testin()[1].x());
    EXPECT_DOUBLE_EQ(rootY, a.testin()[1].y());
}