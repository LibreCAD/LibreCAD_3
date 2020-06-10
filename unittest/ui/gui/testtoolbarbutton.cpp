#include<QApplication>
#include <gtest/gtest.h>

#include <luainterface.h>
#include "apimainwindowtest.h"

#include <widgets/toolbar.h>
#include <widgets/guiAPI/toolbartab.h>
#include <widgets/guiAPI/toolbargroup.h>
#include <widgets/guiAPI/toolbarbutton.h>

#include "../uitests.h"

using namespace lc::ui::widgets;
using namespace lc::ui::api;

TEST(ToolbarButtonTest, LabelAndInitTest) {
    QApplication app(argc, argv);
    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("TestTab");
    ToolbarGroup* testgroup = testtab->addGroup("TestGroup");

    ToolbarButton* testbutton1 = new ToolbarButton("TestButton1", "");
    testgroup->addButton(testbutton1);
    ToolbarButton* testbutton2 = testgroup->addButton("TestButton2", "");

    EXPECT_EQ("TestButton1", testbutton1->label());
    EXPECT_EQ("TestButton2", testbutton2->label());

    EXPECT_EQ(testbutton1, testgroup->buttonByName("TestButton1"));
    EXPECT_EQ(testbutton2, testgroup->buttonByName("TestButton2"));

    testbutton1->setLabel("NewTestButton");

    EXPECT_EQ(nullptr, testgroup->buttonByName("TestButton1"));
    EXPECT_EQ(testbutton1, testgroup->buttonByName("NewTestButton"));
}

TEST(ToolbarButtonTest, TooltipAndRemoveTest) {
    QApplication app(argc, argv);
    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("TestTab");
    ToolbarGroup* testgroup = testtab->addGroup("TestGroup");

    ToolbarButton* testbutton = new ToolbarButton("TestButton1", "", "TestTooltip", true);
    testgroup->addButton(testbutton);

    EXPECT_EQ("TestTooltip", testbutton->toolTip().toStdString());
    EXPECT_TRUE(testbutton->icon().isNull());

    testbutton->setToolTip("NewToolTip");
    EXPECT_EQ("NewToolTip", testbutton->toolTip().toStdString());
    EXPECT_TRUE(testbutton->checkable());

    EXPECT_EQ(testbutton, testgroup->buttonByName("TestButton1"));
    testbutton->remove();
    EXPECT_EQ(nullptr, testgroup->buttonByName("TestButton1"));
}

TEST(ToolbarButtonTest, CallbackTest) {
    QApplication app(argc, argv);
    ApiMainWindowTest* mainWindow = new ApiMainWindowTest();

    lc::ui::LuaInterface* luaInterface = mainWindow->getLuaInterface();
    kaguya::State state(luaInterface->luaState());

    state.dostring("testbutton1 = function() doesThisExist1=true end");
    state.dostring("testbutton2 = function() doesThisExist2=true end");
    kaguya::LuaRef cb1 = state["testbutton1"];
    kaguya::LuaRef cb2 = state["testbutton2"];

    Toolbar* toolbar = mainWindow->getToolbar();
    ToolbarTab* testtab = toolbar->addTab("TestTab");
    ToolbarGroup* testgroup = testtab->addGroup("TestGroup");
    ToolbarButton* testbutton = new ToolbarButton("TestButton", "", cb1);
    testbutton->addCallback(cb2);

    EXPECT_FALSE(state["doesThisExist1"].get<bool>());
    EXPECT_FALSE(state["doesThisExist2"].get<bool>());

    testbutton->pressed();

    // False because button has not been added yet
    EXPECT_FALSE(state["doesThisExist1"].get<bool>());
    EXPECT_FALSE(state["doesThisExist2"].get<bool>());

    testgroup->addButton(testbutton);
    testbutton->pressed();

    EXPECT_TRUE(state["doesThisExist1"].get<bool>());
    EXPECT_TRUE(state["doesThisExist2"].get<bool>());

    EXPECT_EQ(cb2, testbutton->getCallback(1));
}
