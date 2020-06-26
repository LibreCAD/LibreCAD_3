#include<QApplication>
#include <gtest/gtest.h>
#include <vector>

#include "../uitests.h"

#include <widgets/guiAPI/dialogwidget.h>
#include <widgets/guiAPI/inputgui.h>
#include <widgets/guiAPI/anglegui.h>
#include <widgets/guiAPI/textgui.h>
#include <widgets/guiAPI/slidergui.h>
#include <widgets/guiAPI/numbergui.h>
#include <widgets/guiAPI/coordinategui.h>
#include <widgets/guiAPI/colorgui.h>
#include <widgets/guiAPI/comboboxgui.h>
#include <widgets/guiAPI/buttongui.h>
#include <widgets/guiAPI/horizontalgroupgui.h>
#include <widgets/guiAPI/radiogroupgui.h>
#include <widgets/guiAPI/radiobuttongui.h>
#include <kaguya/kaguya.hpp>

TEST(InputGUIWidgetsTest, TestGetLuaValue) {
    QApplication app(argc, argv);
    lc::ui::api::AngleGUI angleGUI("TestAngle");
    angleGUI.setKey("testangle");
    lc::ui::api::TextGUI textGUI("TestText");
    textGUI.setKey("testtext");
    lc::ui::api::SliderGUI sliderGUI("TestSlider");
    sliderGUI.setKey("testslider");
    lc::ui::api::NumberGUI numberGUI("TestNumber");
    numberGUI.setKey("testnumber");
    lc::ui::api::CoordinateGUI coordinateGUI("TestCoordinate");
    coordinateGUI.setKey("testcoordinate");
    lc::ui::api::ColorGUI colorGUI("TestColor");
    colorGUI.setKey("testcolor");
    lc::ui::api::ComboBoxGUI comboboxGUI("TestComboBox");
    comboboxGUI.setKey("testcombobox");

    angleGUI.setValue(10);
    textGUI.setValue("TEST_TEXT");
    sliderGUI.setValue(10);
    numberGUI.setValue(10);
    coordinateGUI.setValue(lc::geo::Coordinate(5,5));
    lc::Color actualcolor = lc::Color(1, 1, 1);
    colorGUI.setValue(actualcolor);
    comboboxGUI.addItem("Item1");
    comboboxGUI.addItem("Item2");
    comboboxGUI.setValue("Item1");

    std::vector<lc::ui::api::InputGUI*> inputgui_list;

    inputgui_list.push_back(&angleGUI);
    inputgui_list.push_back(&textGUI);
    inputgui_list.push_back(&sliderGUI);
    inputgui_list.push_back(&numberGUI);
    inputgui_list.push_back(&coordinateGUI);
    inputgui_list.push_back(&colorGUI);
    inputgui_list.push_back(&comboboxGUI);

    kaguya::State state;
    state["testtable"] = kaguya::NewTable();
    kaguya::LuaRef table = state["testtable"];

    for (lc::ui::api::InputGUI* inputgui : inputgui_list) {
        inputgui->getLuaValue(table);
    }

    double diff = state["testtable"]["testangle"].get<double>() - 10;
    EXPECT_TRUE(diff < 0.01 || diff < -0.01);
    EXPECT_EQ("TEST_TEXT", state["testtable"]["testtext"].get<std::string>());
    EXPECT_EQ(10, state["testtable"]["testslider"].get<int>());
    double diff1 = state["testtable"]["testnumber"].get<double>() - 10;
    EXPECT_TRUE(diff1 < 0.001 || diff1 < -0.001);
    EXPECT_EQ(lc::geo::Coordinate(5, 5), state["testtable"]["testcoordinate"].get<lc::geo::Coordinate>());
    lc::Color testcol = state["testtable"]["testcolor"].get<lc::Color>();
    EXPECT_TRUE(testcol.red() == actualcolor.red() && testcol.green() == actualcolor.green() && testcol.blue() == actualcolor.blue());
    EXPECT_EQ("Item1", state["testtable"]["testcombobox"].get<std::string>());
}

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

TEST(InputGUIWidgetsTest, TextAndNumberGUITest) {
    QApplication app(argc, argv);
    lc::ui::api::TextGUI textGUI("TestText");
    lc::ui::api::NumberGUI numberGUI("TestNumber");

    kaguya::State state;
    state.dostring("cb1 = function(tex) testValue=tex end");
    state.dostring("cb2 = function(num) testValue2=num end");
    textGUI.addOnChangeCallback(state["cb1"]);
    numberGUI.addCallback(state["cb2"]);

    textGUI.setValue("NewValue");
    EXPECT_EQ("NewValue", textGUI.value());
    EXPECT_EQ("NewValue", state["testValue"].get<std::string>());

    numberGUI.setValue(4.8);
    double diff1 = state["testValue2"].get<double>() - 4.8;
    double diff2 = numberGUI.value() - 4.8;
    EXPECT_TRUE(diff1 < 0.01 || diff1 < 0.01);
    EXPECT_TRUE(diff2 < 0.01 || diff2 < 0.01);
}

TEST(InputGUIWidgetsTest, HorizontalGroupTest) {
    QApplication app(argc, argv);
    lc::ui::api::HorizontalGroupGUI horizGroupGUI("TestGroup");

    lc::ui::api::TextGUI textGUI("TestText");
    lc::ui::api::NumberGUI numberGUI("TestNumber");
    lc::ui::api::ButtonGUI buttonGUI("TestButton");

    horizGroupGUI.addWidget("testtext", &textGUI);
    horizGroupGUI.addWidget("testnumber", &numberGUI);
    horizGroupGUI.addWidget("testbutton", &buttonGUI);

    textGUI.setValue("TEST");
    numberGUI.setValue(5);

    std::set<std::string> keysSet = horizGroupGUI.getKeys();
    EXPECT_TRUE(keysSet.find("testtext") != keysSet.end());
    EXPECT_TRUE(keysSet.find("testnumber") != keysSet.end());
    EXPECT_FALSE(keysSet.find("testbutton") != keysSet.end());

    kaguya::State state;
    state["testtable"] = kaguya::NewTable();
    kaguya::LuaRef table = state["testtable"];
    
    horizGroupGUI.getLuaValue(table);
    EXPECT_EQ("TEST", state["testtable"]["testtext"].get<std::string>());
    double diff1 = state["testtable"]["testnumber"].get<double>() - 5;
    EXPECT_TRUE(diff1 < 0.01 || diff1 < 0.01);
}

TEST(InputGUIWidgetsTest, RadioGroupTest) {
    QApplication app(argc, argv);
    lc::ui::api::RadioGroupGUI radioGroupGUI("TestRadioGroup");

    lc::ui::api::RadioButtonGUI radioButton1("RadioButton1");
    lc::ui::api::RadioButtonGUI radioButton2("RadioButton2");

    radioGroupGUI.addButton("button1", &radioButton1);
    radioGroupGUI.addButton("button2", &radioButton2);

    std::set<std::string> keysSet = radioGroupGUI.getKeys();
    EXPECT_TRUE(keysSet.find("button1") != keysSet.end());
    EXPECT_TRUE(keysSet.find("button2") != keysSet.end());

    radioButton1.setValue(true);
    radioButton2.setValue(true);

    kaguya::State state;
    state["testtable"] = kaguya::NewTable();
    kaguya::LuaRef table = state["testtable"];

    radioGroupGUI.getLuaValue(table);
    EXPECT_FALSE(state["testtable"]["button1"].get<bool>());
    EXPECT_TRUE(state["testtable"]["button2"].get<bool>());
}
