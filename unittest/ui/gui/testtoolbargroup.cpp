#include<QApplication>
#include <gtest/gtest.h>

#include <widgets/toolbar.h>
#include <widgets/guiAPI/toolbartab.h>
#include <widgets/guiAPI/toolbargroup.h>
#include <widgets/guiAPI/toolbarbutton.h>

#include "../uitests.h"

using namespace lc::ui::widgets;
using namespace lc::ui::api;

TEST(ToolbarGroupTest, LabelAndInitTest) {
    QApplication app(argc, argv);
    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("TestTab");

    ToolbarGroup* testgroup1 = testtab->addGroup("TestGroup1");
    ToolbarGroup* testgroup2 = new ToolbarGroup("TestGroup2", 4);
    testtab->addGroup(testgroup2);

    EXPECT_EQ("TestGroup1", testgroup1->label());
    EXPECT_EQ("TestGroup2", testgroup2->label());

    testgroup2->setLabel("NewTestGroup");

    EXPECT_EQ("NewTestGroup", testgroup2->label());
    EXPECT_EQ(nullptr, testtab->groupByName("TestGroup2"));
    EXPECT_EQ(testgroup2, testtab->groupByName("NewTestGroup"));
}

TEST(ToolbarGroupTest, ButtonTest) {
    QApplication app(argc, argv);
    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("TestTab");
    ToolbarGroup* testgroup = testtab->addGroup("TestGroup");

    ToolbarButton* button1 = testgroup->addButton("TestButton1", "");
    ToolbarButton* button2 = testgroup->addButton("TestButton2", "");
    ToolbarButton* button3 = new ToolbarButton("TestButton3", "");
    testgroup->addButton(button3);

    EXPECT_EQ(button1, testgroup->buttonByName("TestButton1"));
    EXPECT_EQ(button2, testgroup->buttonByName("TestButton2"));
    EXPECT_EQ(button3, testgroup->buttonByName("TestButton3"));
    std::vector<ToolbarButton*> buttons = testgroup->buttons();

    EXPECT_EQ(3, buttons.size());
    testgroup->removeButton(button1);
    testgroup->removeButton("TestButton2");
    testgroup->removeButton("NonExistentButton");

    buttons = testgroup->buttons();
    EXPECT_EQ(1, buttons.size());
}

TEST(ToolbarGroupTest, RemoveTest) {
    QApplication app(argc, argv);
    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("TestTab");
    ToolbarGroup* testgroup = testtab->addGroup("TestGroup");
    ToolbarButton* button1 = testgroup->addButton("TestButton1", "");
    ToolbarButton* button2 = testgroup->addButton("TestButton2", "");

    EXPECT_EQ(button1, testgroup->buttonByName("TestButton1"));
    EXPECT_EQ(button2, testgroup->buttonByName("TestButton2"));

    testgroup->removeButton(button1);
    testgroup->removeButton("TestButton2");

    EXPECT_EQ(nullptr, testgroup->buttonByName("TestButton1"));
    EXPECT_EQ(nullptr, testgroup->buttonByName("TestButton2"));

    EXPECT_EQ(testgroup, testtab->groupByName("TestGroup"));
    testgroup->remove();
    EXPECT_EQ(nullptr, testtab->groupByName("TestGroup"));
}
