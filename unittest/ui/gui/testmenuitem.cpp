#include<QApplication>
#include <gtest/gtest.h>

#include <mainwindow.h>

#include "../uitests.h"

TEST(MenuItemTest, ItemAddTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::api::Menu* testMenu = mainWindow->addMenu("TESTMENU");
    lc::ui::api::MenuItem* testItem1 = testMenu->addItem("TESTITEM1");

    EXPECT_EQ("TESTITEM1", testItem1->label());

    lc::ui::api::MenuItem* testItem2 = new lc::ui::api::MenuItem("TESTITEM2");
    testMenu->addItem(testItem2);

    EXPECT_EQ(testItem2, testMenu->itemByName("TESTITEM2"));

    lc::ui::api::MenuItem* testItem3 = new lc::ui::api::MenuItem("TESTITEM3", kaguya::LuaRef());
    testMenu->addItem(testItem3);

    EXPECT_EQ(testItem3, testMenu->itemByName("TESTITEM3"));

    testMenu->addItem("TESTITEM4", kaguya::LuaRef());
    lc::ui::api::MenuItem* testItem4 = testMenu->itemByName("TESTITEM4");

    EXPECT_EQ("TESTITEM4", testItem4->label());
}

TEST(MenuItemTest, ItemLabelTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::api::MenuItem* testItem1 = new lc::ui::api::MenuItem("LABEL_TEST");
    
    EXPECT_EQ("LABEL_TEST", testItem1->label());

    testItem1->setLabel("CHANGED_LABEL");

    EXPECT_EQ("CHANGED_LABEL", testItem1->label());
}

TEST(MenuItemTest, ItemCallbackTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();
    
    lc::ui::LuaInterface* luaInterface = mainWindow->luaInterface();
    kaguya::State state(luaInterface->luaState());

    state.dostring("testitemcb = function() doesThisExist=true end");
    kaguya::LuaRef cb = state["testitemcb"];

    lc::ui::api::Menu* testmenu = mainWindow->addMenu("testmenu");
    lc::ui::api::MenuItem* testitem1 = new lc::ui::api::MenuItem("TestItem1", cb);
    testmenu->addItem(testitem1);

    testitem1->trigger();

    EXPECT_TRUE(state["doesThisExist"].get<bool>());

    state.dostring("testitemcb2 = function() doesThisExist2=true end");
    state.dostring("testitemcb3 = function() doesThisExist3=true end");

    kaguya::LuaRef cb2 = state["testitemcb2"];
    kaguya::LuaRef cb3 = state["testitemcb3"];

    lc::ui::api::MenuItem* testitem2 = testmenu->addItem("TestItem2", cb2);
    testitem2->addCallback(cb3);

    EXPECT_FALSE(state["doesThisExist2"].get<bool>());
    EXPECT_FALSE(state["doesThisExist3"].get<bool>());

    testitem2->trigger();

    EXPECT_TRUE(state["doesThisExist2"].get<bool>());
    EXPECT_TRUE(state["doesThisExist3"].get<bool>());
}

TEST(MenuItemTest, ItemPositionTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::api::Menu* testmenu = mainWindow->addMenu("TestMenu");
    lc::ui::api::MenuItem* testitem1 = testmenu->addItem("TestItem1");
    lc::ui::api::MenuItem* testitem2 = testmenu->addItem("TestItem2");
    lc::ui::api::MenuItem* testitem3 = testmenu->addItem("TestItem3");

    EXPECT_EQ(0, testitem1->position());
    EXPECT_EQ(1, testitem2->position());
    EXPECT_EQ(2, testitem3->position());

    testitem3->setPosition(0);

    EXPECT_EQ(1, testitem1->position());
    EXPECT_EQ(2, testitem2->position());
    EXPECT_EQ(0, testitem3->position());

    testitem2->setPosition(testitem3->position());

    EXPECT_EQ(2, testitem1->position());
    EXPECT_EQ(0, testitem2->position());
    EXPECT_EQ(1, testitem3->position());

    testitem1->setPosition(2);

    EXPECT_EQ(2, testitem1->position());
    EXPECT_EQ(0, testitem2->position());
    EXPECT_EQ(1, testitem3->position());

    testitem3->setPosition(4);

    EXPECT_EQ(1, testitem1->position());
    EXPECT_EQ(0, testitem2->position());
    EXPECT_EQ(2, testitem3->position());
}

TEST(MenuItemTest, ItemRemoveTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::api::Menu* testmenu = mainWindow->addMenu("TestMenu");
    lc::ui::api::MenuItem* testitem1 = testmenu->addItem("TestItem1");
    lc::ui::api::MenuItem* testitem2 = testmenu->addItem("TestItem2");
    lc::ui::api::MenuItem* testitem3 = testmenu->addItem("TestItem3");

    EXPECT_EQ(0, testitem1->position());
    EXPECT_EQ(1, testitem2->position());
    EXPECT_EQ(2, testitem3->position());

    testitem1->remove();

    EXPECT_EQ(0, testitem2->position());
    EXPECT_EQ(1, testitem3->position());

    lc::ui::api::MenuItem* testitem4 = testmenu->addItem("TestItem4");

    EXPECT_EQ(0, testitem2->position());
    EXPECT_EQ(1, testitem3->position());
    EXPECT_EQ(2, testitem4->position());

    testitem2->setPosition(testitem4->position());

    EXPECT_EQ(2, testitem2->position());
    EXPECT_EQ(0, testitem3->position());
    EXPECT_EQ(1, testitem4->position());

    testitem4->remove();

    EXPECT_EQ(1, testitem2->position());
    EXPECT_EQ(0, testitem3->position());
}
