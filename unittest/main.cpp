#include "code.h"
#include "gtest/gtest.h"
using namespace std;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


TEST(test, quad) {
  test c;

  EXPECT_DOUBLE_EQ (-2, c.quad(5,6)[0]);
  EXPECT_DOUBLE_EQ (-3, c.quad(5,6)[1]);
}

TEST(test, cubic) {
  test c;

  EXPECT_DOUBLE_EQ (-0.19806226419516171, c.cubic(5,6,1)[0]);
  EXPECT_DOUBLE_EQ (-1.5549581320873713, c.cubic(5,6,1)[1]);
  EXPECT_DOUBLE_EQ (-3.2469796037174667, c.cubic(5,6,1)[2]);
}

TEST(test, quartic) {
  test c;

  EXPECT_DOUBLE_EQ (-2.0, c.quartic(5,6,1,2)[0]);
  EXPECT_DOUBLE_EQ (-3.1038034027355366, c.quartic(5,6,1,2)[1]);
}

TEST(test, testin) {
  test a;
  EXPECT_DOUBLE_EQ(0, a.testin()[1].x());
}
