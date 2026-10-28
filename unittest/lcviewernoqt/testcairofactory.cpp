#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <vector>
#include "painters/createpainter.h"

// Cairo ARGB32 uses native-endian 32-bit words; it is not RHI RGBA8 readback.
TEST(ViewerFactoryTest, CairoOwnsCpuPixels) {
    std::vector<std::uint32_t> pixels(7 * 5, 0);
    {
        std::unique_ptr<lc::viewer::LcPainter> painter(lc::viewer::createCairoImagePainter(
            reinterpret_cast<unsigned char*>(pixels.data()), 7, 5));
        ASSERT_NE(nullptr, painter);
        painter->clear(1.0, 0.0, 0.0, 1.0);
    }
    for (auto pixel : pixels) {
        EXPECT_EQ(UINT32_C(0xffff0000), pixel);
    }
}
