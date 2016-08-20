#include <gtest/gtest.h>
#include "uitests.h"

#include <widgets/linewidthselect.h>

TEST(LineWidthSelectTest, Selection) {
    QApplication app(argc, argv);
    LineWidthSelect lineWidthSelect(0, false, false);
    LineWidthSelect lineWidthSelect_ByBlock_ByLayer(0, true, true);

    /*
     * Test number of entries
     */
    EXPECT_NE(0, lineWidthSelect.count()) << "Line width select doesn't contain any element";
    EXPECT_EQ(lineWidthSelect.count() + 2, lineWidthSelect_ByBlock_ByLayer.count()) << "Line width select doesn't contain ByLayer or ByLayer";

    /*
     * Test custom width
     */
    auto width = 0.13;
    lineWidthSelect.setWidth(width);
    EXPECT_EQ(width, lineWidthSelect.lineWidth()->width());
}