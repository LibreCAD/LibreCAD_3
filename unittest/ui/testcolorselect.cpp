#include <gtest/gtest.h>
#include "uitests.h"

#include <widgets/colorselect.h>

TEST(ColorSelectTest, Selection) {
    QApplication app(argc, argv);
    lc::ui::widgets::ColorSelect colorSelect(nullptr, 0, false, false);
    lc::ui::widgets::ColorSelect colorSelect_ByBlock_ByLayer(nullptr, 0, true, true);
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
    auto lcMetaColor = lc::meta::MetaColorByValue(lcColor);
    colorSelect.setCurrentText("red");

    auto colorByValue = std::dynamic_pointer_cast<const lc::meta::MetaColorByValue>(colorSelect.metaColor());

    EXPECT_TRUE(
            lcColor.red() == colorSelect.color().red() &&
            lcColor.green() == colorSelect.color().green() &&
            lcColor.blue() == colorSelect.color().blue()
    );
    EXPECT_TRUE(
            lcMetaColor.red() == colorByValue->red() &&
            lcMetaColor.green() == colorByValue->green() &&
            lcMetaColor.blue() == colorByValue->blue()
    );
}

TEST(ColorSelectTest, CustomColor) {
    QApplication app(argc, argv);
    lc::ui::widgets::ColorSelect colorSelect(nullptr, 0, false, false);
    lc::Color lcColor(1, 2, 3, 4);
    lc::meta::MetaColorByValue lcMetaColor(lcColor);

    colorSelect.setColor(lcColor);

    auto colorByValue = std::dynamic_pointer_cast<const lc::meta::MetaColorByValue>(colorSelect.metaColor());

    EXPECT_TRUE(
            lcColor.red() == colorSelect.color().red() &&
            lcColor.green() == colorSelect.color().green() &&
            lcColor.blue() == colorSelect.color().blue()
    );
    EXPECT_TRUE(
            lcMetaColor.red() == colorByValue->red() &&
            lcMetaColor.green() == colorByValue->green() &&
            lcMetaColor.blue() == colorByValue->blue()
    );
}