#include <cad/math/floatingpoint.h>

#include <gtest/gtest.h>

#include <cmath>
#include <limits>

using lc::maths::compareWithinUlps;
using lc::maths::FloatingOrder;
using lc::maths::FloatingSign;
using lc::maths::signWithinUlpsOfZero;

namespace {
double step(double value, double direction, unsigned int count) {
    while (count--) value = std::nextafter(value, direction);
    return value;
}
}

TEST(FloatingPointTest, RespectsUlpBudgetOnBothSides) {
    const double infinity = std::numeric_limits<double>::infinity();
    for (double boundary : {1.0, -1.0, 0.0, std::numeric_limits<double>::min()}) {
        const double below = step(boundary, -infinity, 4);
        const double above = step(boundary, infinity, 4);
        EXPECT_EQ(FloatingOrder::Equivalent, compareWithinUlps(below, boundary));
        EXPECT_EQ(FloatingOrder::Equivalent, compareWithinUlps(above, boundary));
        EXPECT_EQ(FloatingOrder::Less, compareWithinUlps(step(boundary, -infinity, 5), boundary));
        EXPECT_EQ(FloatingOrder::Greater, compareWithinUlps(step(boundary, infinity, 5), boundary));
        EXPECT_EQ(FloatingOrder::Equivalent, compareWithinUlps(boundary, below));
        EXPECT_EQ(FloatingOrder::Equivalent, compareWithinUlps(boundary, above));
        EXPECT_EQ(FloatingOrder::Less, compareWithinUlps(boundary, step(boundary, infinity, 5)));
        EXPECT_EQ(FloatingOrder::Greater, compareWithinUlps(boundary, step(boundary, -infinity, 5)));
    }
    EXPECT_EQ(FloatingOrder::Greater, compareWithinUlps(std::nextafter(1.0, infinity), 1.0, 0));

    const double sixtyFourUlpsAboveOne = step(1.0, infinity, 64);
    EXPECT_EQ(FloatingOrder::Equivalent, compareWithinUlps(sixtyFourUlpsAboveOne, 1.0, 64));
    EXPECT_EQ(FloatingOrder::Greater, compareWithinUlps(sixtyFourUlpsAboveOne, 1.0, 63));
}

TEST(FloatingPointTest, HandlesZeroSubnormalsAndSigns) {
    const double infinity = std::numeric_limits<double>::infinity();
    EXPECT_EQ(FloatingOrder::Equivalent, compareWithinUlps(-0.0, 0.0, 0));
    EXPECT_EQ(FloatingSign::Zero, signWithinUlpsOfZero(step(0.0, infinity, 4)));
    EXPECT_EQ(FloatingSign::Zero, signWithinUlpsOfZero(step(0.0, -infinity, 4)));
    EXPECT_EQ(FloatingSign::Positive, signWithinUlpsOfZero(step(0.0, infinity, 5)));
    EXPECT_EQ(FloatingSign::Negative, signWithinUlpsOfZero(step(0.0, -infinity, 5)));
    EXPECT_EQ(FloatingSign::Positive, signWithinUlpsOfZero(std::numeric_limits<double>::denorm_min(), 0));
    EXPECT_EQ(FloatingOrder::Equivalent,
              compareWithinUlps(std::numeric_limits<double>::denorm_min(),
                                -std::numeric_limits<double>::denorm_min(), 2));
    EXPECT_EQ(FloatingOrder::Greater,
              compareWithinUlps(std::numeric_limits<double>::denorm_min(),
                                -std::numeric_limits<double>::denorm_min(), 1));
}

TEST(FloatingPointTest, KeepsNonFiniteValuesDistinct) {
    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double largest = std::numeric_limits<double>::max();
    EXPECT_EQ(FloatingOrder::Unordered, compareWithinUlps(nan, 1.0));
    EXPECT_EQ(FloatingOrder::Unordered, compareWithinUlps(1.0, nan));
    EXPECT_EQ(FloatingOrder::Unordered, compareWithinUlps(nan, nan));
    EXPECT_EQ(FloatingOrder::Equivalent, compareWithinUlps(infinity, infinity));
    EXPECT_EQ(FloatingOrder::Equivalent, compareWithinUlps(-infinity, -infinity));
    EXPECT_EQ(FloatingOrder::Less, compareWithinUlps(-infinity, infinity));
    EXPECT_EQ(FloatingOrder::Less, compareWithinUlps(largest, infinity));
    EXPECT_EQ(FloatingOrder::Greater, compareWithinUlps(infinity, largest));
    EXPECT_EQ(FloatingSign::NonFinite, signWithinUlpsOfZero(infinity));
    EXPECT_EQ(FloatingSign::NonFinite, signWithinUlpsOfZero(nan));
}
