#include<QApplication>
#include <gtest/gtest.h>

#include <widgets/guiAPI/dialogwidget.h>
#include <widgets/guiAPI/inputgui.h>
#include <widgets/guiAPI/buttongui.h>
#include <widgets/guiAPI/checkboxgui.h>
#include <widgets/guiAPI/anglegui.h>
#include <widgets/guiAPI/slidergui.h>
#include <widgets/guiAPI/textgui.h>

#include "../uitests.h"

TEST(DialogWidgetTest, AddTest) {
    QApplication app(argc, argv);
    lc::ui::api::DialogWidget dialogWidget("DialogTitle", nullptr);

    EXPECT_EQ("DialogTitle", dialogWidget.title());
    dialogWidget.setTitle("TestTitle");
    EXPECT_EQ("TestTitle", dialogWidget.title());

    lc::ui::api::ButtonGUI button1("TestButton");
    lc::ui::api::AngleGUI angle1("TestAngle");
    lc::ui::api::AngleGUI angle2("TestAngle2");
    lc::ui::api::CheckBoxGUI check1("TestCheckBox");

    bool success1 = dialogWidget.addWidget("testbutton", &button1);
    bool success2 = dialogWidget.addWidget("testangle", &angle1);
    bool success3 = dialogWidget.addWidget("testangle", &angle2);
    bool success4 = dialogWidget.addWidget("testcheckbox", &check1);

    EXPECT_TRUE(success1);
    EXPECT_TRUE(success2);
    EXPECT_FALSE(success3);
    EXPECT_TRUE(success4);

    const std::vector<lc::ui::api::InputGUI*>& guiWidgets = dialogWidget.inputWidgets();
    for (lc::ui::api::InputGUI* gui : guiWidgets) {
        EXPECT_TRUE(gui->label() == "TestAngle" || gui->label() == "testbutton_group" || gui->label() == "testcheckbox_group");
    }

    std::vector<std::string> keysList = dialogWidget.keys();
    for (std::string key : keysList) {
        EXPECT_TRUE(key == "testangle" || key == "testbutton_group" || key == "testcheckbox_group" || key == "testcheckbox" || key == "testbutton");
    }
}

TEST(DialogWidgetTest, CallbackTest) {
    QApplication app(argc, argv);
    lc::ui::MainWindow mainWindow;
    lc::ui::api::DialogWidget dialogWidget("DialogTitle", &mainWindow);

    lc::ui::api::TextGUI text1("TestText");
    lc::ui::api::SliderGUI slider1("TestSlider");
    lc::ui::api::ButtonGUI button1("TestButton");

    dialogWidget.addWidget("testtext", &text1);
    dialogWidget.addWidget("testslider", &slider1);
    dialogWidget.addWidget("testbutton", &button1);

    dialogWidget.setFinishButton(&button1);

    text1.setValue("TEST");
    slider1.setValue(5);

    kaguya::State state(mainWindow.luaInterface()->luaState());
    state.dostring("cb1 = function(tab) valuetable=tab end");

    dialogWidget.addFinishCallback(state["cb1"]);
    button1.clicked();

    EXPECT_EQ("TEST", state["valuetable"]["testtext"].get<std::string>());
    EXPECT_EQ(5, state["valuetable"]["testslider"].get<int>());
}
