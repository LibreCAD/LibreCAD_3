#include<QApplication>
#include<QSignalSpy>
#include <gtest/gtest.h>

#include <widgets/toolbar.h>
#include <widgets/guiAPI/toolbartab.h>
#include <widgets/guiAPI/toolbargroup.h>

#include "../uitests.h"

using namespace lc::ui::widgets;
using namespace lc::ui::api;

TEST(ToolbarTabTest, LabelAndInitTest) {
    QApplication app(argc, argv);
    Toolbar toolbar(nullptr);

    ToolbarTab* testtab1 = toolbar.addTab("TestTab1");
    ToolbarTab* testtab2 = new ToolbarTab("TestTab2");
    toolbar.addTab(testtab2);

    EXPECT_EQ("TestTab1", testtab1->label());
    EXPECT_EQ("TestTab2", testtab2->label());

    QSignalSpy spy(testtab2, SIGNAL(labelChanged(QString,QString)));

    testtab2->setLabel("NewTestTab");

    EXPECT_EQ("NewTestTab", testtab2->label());
    EXPECT_EQ(1, spy.count());

    QList<QVariant> arguments = spy.takeFirst();
    std::string arg0 = arguments.at(0).toString().toStdString();
    std::string arg1 = arguments.at(1).toString().toStdString();
    std::string expected0 = "NewTestTab";
    std::string expected1 = "TestTab2";
    EXPECT_EQ(arg0, expected0);
    EXPECT_EQ(arg1, expected1);

    EXPECT_EQ(testtab1, toolbar.tabByName("TestTab1"));
    EXPECT_EQ(testtab2, toolbar.tabByName("NewTestTab"));
}

TEST(ToolbarTabTest, GroupTest) {
    QApplication app(argc, argv);
    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("TestTab");

    ToolbarGroup* testgroup1 = testtab->addGroup("TestGroup1");
    ToolbarGroup* testgroup2 = new ToolbarGroup("TestGroup2");
    testtab->addGroup(testgroup2);

    EXPECT_EQ(testgroup1, testtab->groupByName("TestGroup1"));
    EXPECT_EQ(testgroup2, testtab->groupByName("TestGroup2"));

    testtab->addGroup("TestGroup2");
    std::vector<ToolbarGroup*> allgroups = testtab->groups();

    EXPECT_EQ(2, allgroups.size());
    testtab->removeGroup("TestGroup2");
    testtab->removeGroup(testgroup1);
    allgroups = testtab->groups();
    EXPECT_EQ(0, allgroups.size());
}

TEST(ToolbarTabTest, RemoveTest) {
    QApplication app(argc, argv);
    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("TestTab");

    ToolbarGroup* testgroup1 = testtab->addGroup("TestGroup1");
    ToolbarGroup* testgroup2 = testtab->addGroup("TestGroup2");
    ToolbarGroup* testgroup3 = testtab->addGroup("TestGroup3");
    ToolbarGroup* testgroup4 = testtab->addGroup("TestGroup4");

    EXPECT_EQ(testgroup1, testtab->groupByName("TestGroup1"));
    EXPECT_EQ(testgroup2, testtab->groupByName("TestGroup2"));
    EXPECT_EQ(testgroup3, testtab->groupByName("TestGroup3"));
    EXPECT_EQ(testgroup4, testtab->groupByName("TestGroup4"));

    testgroup1->remove();
    testtab->removeGroup("TestGroup2");
    testtab->removeGroup(testgroup3);
    testtab->removeGroup("TestGroup5");

    EXPECT_EQ(nullptr, testtab->groupByName("TestGroup1"));
    EXPECT_EQ(nullptr, testtab->groupByName("TestGroup2"));
    EXPECT_EQ(nullptr, testtab->groupByName("TestGroup3"));
    EXPECT_EQ(testgroup4, testtab->groupByName("TestGroup4"));
}
