#include <gtest/gtest.h>
#include "uitests.h"

#include <widgets/linewidthselect.h>

TEST(LineWidthSelectTest, Selection) {
    QApplication app(argc, argv);
    lc::ui::LineWidthSelect lineWidthSelect(nullptr, 0, false, false);
    lc::ui::LineWidthSelect lineWidthSelect_ByBlock_ByLayer(nullptr, 0, true, true);

    /*
     * Test number of entries
     */
    EXPECT_NE(0, lineWidthSelect.count()) << "Line width select doesn't contain any element";
    EXPECT_EQ(lineWidthSelect.count() + 2, lineWidthSelect_ByBlock_ByLayer.count()) << "Line width select doesn't contain ByLayer or ByLayer";

    /*
     * Test custom width
     */
    auto width = 0.13;
    lineWidthSelect.setWidth(std::make_shared<lc::MetaLineWidthByValue>(width));
    EXPECT_EQ(width, std::dynamic_pointer_cast<const lc::MetaLineWidthByValue>(lineWidthSelect.lineWidth())->width());
}