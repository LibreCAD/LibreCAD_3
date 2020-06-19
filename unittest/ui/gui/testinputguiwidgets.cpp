#include<QApplication>
#include <gtest/gtest.h>

#include "../uitests.h"

#include <widgets/guiAPI/dialogwidget.h>
#include <widgets/guiAPI/inputgui.h>
#include <widgets/guiAPI/anglegui.h>
#include <kaguya/kaguya.hpp>

TEST(InputGUIWidgetsTest, AngleGUITest) {
    QApplication app(argc, argv);
    lc::ui::api::DialogWidget dialogWidget("TestDialog", nullptr);
    lc::ui::api::AngleGUI angleGUI("TestAngle");

    EXPECT_EQ("TestAngle", angleGUI.label());
    angleGUI.setLabel("NewAngle");
    EXPECT_EQ("NewAngle", angleGUI.label());

    angleGUI.setValue(3.14);
    float diff = angleGUI.value() - 3.14;
    EXPECT_TRUE(diff < 0.01 || diff < -0.01);

    kaguya::State state;
    state.dostring("cb1 = function(ang) testValue=ang end");
    angleGUI.addOnChangeCallback(state["cb1"]);
    angleGUI.setValue(5);

    double diff1 = state["testValue"].get<double>() - 5;
    EXPECT_TRUE(diff1 < 0.01 || diff1 < 0.01);
}
