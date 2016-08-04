/*
 * All unit tests should be written in a new file and added to CMakeLists.txt.
 * Nothing should be included here except gtest.h to prevent recompilation of a single test taking too much time.
 */

#include <gtest/gtest.h>

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}