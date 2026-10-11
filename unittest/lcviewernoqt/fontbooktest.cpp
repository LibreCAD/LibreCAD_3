#include <gtest/gtest.h>
#include "painters/opengl/font_book.h"

TEST(FontBookTest, CopyAssignmentPreservesDefaultLookup) {
    lc::viewer::opengl::Font_Book source;
    lc::viewer::opengl::Font_Book copy;
    copy = source;

    EXPECT_TRUE(copy.getFontList().empty());
    EXPECT_EQ(nullptr, copy.pickFont("missing font"));
}
