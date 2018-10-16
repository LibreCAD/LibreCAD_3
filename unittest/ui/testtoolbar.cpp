#include <gtest/gtest.h>

#include <QApplication>

#include <widgets/toolbar.h>
#include <widgets/toolbartab.h>

static int argc = 0;
static char** argv = NULL;

using namespace lc::ui::widgets;

TEST(ToolbarTest, TabOperations) {
	QApplication app(argc, argv);
	Toolbar toolbar;

	toolbar.addTab("Test", new ToolbarTab());
	auto tab = toolbar.tabByName("Test");

	EXPECT_NE(nullptr, tab);

	toolbar.removeTab(tab);
	tab = toolbar.tabByName("Test");

	EXPECT_EQ(nullptr, tab);
}

TEST(ToolbarTest, GroupOperations) {
	QApplication app(argc, argv);
	ToolbarTab toolbarTab;

	toolbarTab.addGroup("Test");
	auto group = toolbarTab.groupByName("Test");

	EXPECT_NE(nullptr, group);

	toolbarTab.removeGroup(group);
	group = toolbarTab.groupByName("Test");

	EXPECT_EQ(nullptr, group);
}

TEST(ToolbarTest, ButtonOperations) {
	QApplication app(argc, argv);
	ToolbarTab toolbarTab;

	toolbarTab.addGroup("Test");
	auto group = toolbarTab.groupByName("Test");

	toolbarTab.addButton(group, "TestButton");
	auto button = toolbarTab.buttonByText(group, "TestButton");

	EXPECT_NE(nullptr, button);

	toolbarTab.removeButton(button);
	button = toolbarTab.buttonByText(group, "TestButton");

	EXPECT_EQ(nullptr, button);
}