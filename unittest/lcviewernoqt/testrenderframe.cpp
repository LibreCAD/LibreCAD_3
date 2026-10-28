#include <gtest/gtest.h>
#include "painters/renderframe.h"
#include <limits>
#include <stdexcept>

namespace {
using lc::viewer::render::Frame;
using lc::viewer::render::validateFrame;
Frame triangle() {
    Frame f; f.width = 65; f.height = 49;
    f.vertices = {{4, 4, 1, 0, 0, 1}, {61, 4, 1, 0, 0, 1}, {4, 45, 1, 0, 0, 1}};
    return f;
}
}
TEST(RenderFrameTest, InvalidFrames) {
    auto f = triangle(); EXPECT_NO_THROW(validateFrame(f));
    f.width = -1; EXPECT_THROW(validateFrame(f), std::invalid_argument);
    f = triangle(); f.width = f.height = 16384;
    EXPECT_THROW(validateFrame(f), std::invalid_argument);
    f = triangle(); f.vertices.pop_back();
    EXPECT_THROW(validateFrame(f), std::invalid_argument);
    f = triangle(); f.clear[0] = std::numeric_limits<float>::quiet_NaN();
    EXPECT_THROW(validateFrame(f), std::invalid_argument);
    f = triangle(); f.vertices[0].a = 0;
    EXPECT_THROW(validateFrame(f), std::invalid_argument);
}
TEST(RenderFrameTest, InvalidIndices) {
    Frame f; f.width = 3; f.height = 2;
    f.vertices = {{0, 0, 1, 0, 0, 1}, {3, 0, 1, 0, 0, 1}, {3, 2, 1, 0, 0, 1}, {0, 2, 1, 0, 0, 1}};
    f.indices = {0, 1, 2, 0, 2, 3};
    EXPECT_NO_THROW(validateFrame(f));
    f.indices.back() = 4;
    EXPECT_THROW(validateFrame(f), std::invalid_argument);
    f.indices.back() = 3; f.indices.pop_back();
    EXPECT_THROW(validateFrame(f), std::invalid_argument);
    f.indices.clear(); // Four vertices are not a non-indexed triangle list.
    EXPECT_THROW(validateFrame(f), std::invalid_argument);
}
TEST(RenderFrameTest, SuspendedFramesHaveNoMesh) {
    Frame f;
    EXPECT_NO_THROW(validateFrame(f));
    f.height = 49;
    EXPECT_NO_THROW(validateFrame(f));
    f.vertices = {{0, 0, 0, 0, 0, 1}};
    EXPECT_THROW(validateFrame(f), std::invalid_argument);
}
