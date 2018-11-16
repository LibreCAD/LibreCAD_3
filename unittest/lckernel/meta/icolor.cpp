#include <gtest/gtest.h>
#include <cad/meta/icolor.h>

using namespace lc;
using namespace meta;

TEST(iColor, colorToInt) {
    iColor icol;

    ASSERT_EQ(icol.colorToInt(lc::Color(0, 0, 0)), 0);
    ASSERT_EQ(icol.colorToInt(lc::Color(2, 2, 2)), 0);
    ASSERT_EQ(icol.colorToInt(lc::Color(254, 254, 254)), 7);
    ASSERT_EQ(icol.colorToInt(lc::Color(255, 255, 255)), 7);
}