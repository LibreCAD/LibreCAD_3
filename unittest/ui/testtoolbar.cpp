#include <gtest/gtest.h>

#include <QApplication>

#include <mainwindow.h>
#include <widgets/toolbar.h>
#include <widgets/guiAPI/toolbartab.h>
#include <widgets/guiAPI/toolbargroup.h>
#include <widgets/guiAPI/toolbarbutton.h>
#include "uitests.h"

using namespace lc::ui::widgets;
using namespace lc::ui::api;

TEST(ToolbarTest, TabOperations) {
	QApplication app(argc, argv);
	Toolbar toolbar(nullptr);

	toolbar.addTab("TestTab1");
    toolbar.addTab("TestTab2");
	auto tab = toolbar.tabByName("TestTab1");

	EXPECT_NE(nullptr, tab);
    EXPECT_NE(nullptr, toolbar.tabByName("TestTab2"));
    EXPECT_EQ("TestTab1", tab->label());

	toolbar.removeTab(tab);
	tab = toolbar.tabByName("TestTab1");

	EXPECT_EQ(nullptr, tab);
    EXPECT_NE(nullptr, toolbar.tabByName("TestTab2"));

    toolbar.removeTab("TestTab2");
    EXPECT_EQ(nullptr, toolbar.tabByName("TestTab2"));
}

TEST(ToolbarTest, GroupOperations) {
	QApplication app(argc, argv);
    Toolbar toolbar(nullptr);

    auto tab1 = toolbar.addTab("TestTab1");
    tab1->addGroup("TestGroup1");
    auto quickaccess = toolbar.addTab("Quick Access");
    quickaccess->addGroup("TestGroup2");

    EXPECT_NE(nullptr, tab1->groupByName("TestGroup1"));
    EXPECT_NE(nullptr, quickaccess->groupByName("TestGroup2"));

    toolbar.removeGroupByName("TestGroup2");

    EXPECT_NE(nullptr, tab1->groupByName("TestGroup1"));
    EXPECT_EQ(nullptr, quickaccess->groupByName("TestGroup2"));

    toolbar.removeGroupByName("TestGroup1", "TestTab1");

    EXPECT_EQ(nullptr, tab1->groupByName("TestGroup1"));
}

TEST(ToolbarTest, SlotTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::LuaInterface* luaInterface = mainWindow->luaInterface();
    kaguya::State state(luaInterface->luaState());

    state.dostring("testtoolbar = function() doesThisExist=true end");
    kaguya::LuaRef cb = state["testtoolbar"];

    Toolbar* toolbar = mainWindow->toolbar();
    ToolbarTab* testtab = toolbar->addTab("TestTab");

    EXPECT_EQ(testtab, toolbar->tabByName("TestTab"));
    testtab->setLabel("NewTab");
    EXPECT_EQ(testtab, toolbar->tabByName("NewTab"));

    ToolbarButton button("TestButton", "");
    button.addCallback(cb);

    EXPECT_FALSE(state["doesThisExist"].get<bool>());
    button.pressed();
    EXPECT_TRUE(state["doesThisExist"].get<bool>());

    mainWindow->close();
}
