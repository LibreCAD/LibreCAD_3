#include <gtest/gtest.h>
#include "uitests.h"

#include <widgets/colorselect.h>

TEST(ColorSelectTest, Selection) {
    QApplication app(argc, argv);
    lc::ui::ColorSelect colorSelect(nullptr, 0, false, false);
    lc::ui::ColorSelect colorSelect_ByBlock_ByLayer(nullptr, 0, true, true);
    auto nbColors = QColor::colorNames().size();

    /*
     * Check for ByBlock and ByLayer (+ 2 for separator and custom color)
     */
    EXPECT_EQ(nbColors + 4, colorSelect_ByBlock_ByLayer.count());;
    EXPECT_EQ(nbColors + 2, colorSelect.count());

    /*
     * Check color selection
     */
    auto lcColor = lc::Color(255, 0, 0, 255);
    auto lcMetaColor = lc::MetaColor(lcColor);
    colorSelect.setCurrentText("red");
    EXPECT_TRUE(
            lcColor.red() == colorSelect.color().red() &&
            lcColor.green() == colorSelect.color().green() &&
            lcColor.blue() == colorSelect.color().blue()
    );
    EXPECT_TRUE(
            lcMetaColor.red() == colorSelect.metaColor()->red() &&
            lcMetaColor.green() == colorSelect.metaColor()->green() &&
            lcMetaColor.blue() == colorSelect.metaColor()->blue()
    );
}

TEST(ColorSelectTest, CustomColor) {
    QApplication app(argc, argv);
    lc::ui::ColorSelect colorSelect(nullptr, 0, false, false);
    lc::Color lcColor(1, 2, 3, 4);
    lc::MetaColor lcMetaColor(lcColor);

    colorSelect.setColor(lcColor);
    EXPECT_TRUE(
            lcColor.red() == colorSelect.color().red() &&
            lcColor.green() == colorSelect.color().green() &&
            lcColor.blue() == colorSelect.color().blue()
    );
    EXPECT_TRUE(
            lcMetaColor.red() == colorSelect.metaColor()->red() &&
            lcMetaColor.green() == colorSelect.metaColor()->green() &&
            lcMetaColor.blue() == colorSelect.metaColor()->blue()
    );
}