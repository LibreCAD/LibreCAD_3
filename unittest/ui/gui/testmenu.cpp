#include<QApplication>
#include <gtest/gtest.h>

#include "apimainwindowtest.h"

#include "../uitests.h"

TEST(MenuTest, MenuLabelTest) {
    QApplication app(argc, argv);
    ApiMainWindowTest* mainWindow = new ApiMainWindowTest();

    lc::ui::api::Menu* testMenu = mainWindow->addMenu("TESTMENU");
    lc::ui::api::Menu* testMenu2 = new lc::ui::api::Menu("TESTMENU2");

    mainWindow->addMenu(testMenu2);

    lc::ui::api::Menu* insideMenu1 = testMenu->addMenu("INSIDEMENU1");

    EXPECT_EQ("TESTMENU", testMenu->getLabel());
    EXPECT_EQ("INSIDEMENU1", insideMenu1->getLabel());

    lc::ui::api::Menu* insideMenu2 = new lc::ui::api::Menu("INSIDEMENU2");
    testMenu->addMenu(insideMenu2);

    EXPECT_EQ("INSIDEMENU2", insideMenu2->getLabel());

    insideMenu2->setLabel("NEWINSIDEMENU2");

    EXPECT_EQ("NEWINSIDEMENU2", insideMenu2->getLabel());
    
    EXPECT_EQ(insideMenu2, testMenu->getMenu("NEWINSIDEMENU2"));
    EXPECT_EQ(nullptr,testMenu->getMenu("INSIDEMENU2"));

    EXPECT_EQ(mainWindow->getMenu("TESTMENU2")->getLabel(), "TESTMENU2");
}

TEST(MenuTest, MenuGetTest) {
    QApplication app(argc, argv);
    ApiMainWindowTest* mainWindow = new ApiMainWindowTest();

    lc::ui::api::Menu* testMenu = mainWindow->addMenu("TESTMENU");
    lc::ui::api::Menu* testMenu2 = new lc::ui::api::Menu("TESTMENU2");

    mainWindow->addMenu(testMenu2);

    lc::ui::api::Menu* insideMenu1 = testMenu->addMenu("INSIDEMENU1");
    lc::ui::api::Menu* insideMenu2 = new lc::ui::api::Menu("INSIDEMENU2");
    testMenu->addMenu(insideMenu2);

    EXPECT_EQ(testMenu, mainWindow->getMenu("TESTMENU"));
    EXPECT_EQ(testMenu2, mainWindow->getMenu("TESTMENU2"));

    EXPECT_EQ(insideMenu1, testMenu->getMenu("INSIDEMENU1"));
    EXPECT_EQ(insideMenu2, testMenu->getMenu("INSIDEMENU2"));

    insideMenu1->setLabel("INSIDEMENU3");

    EXPECT_EQ(nullptr, testMenu->getMenu("INSIDEMENU1"));
    EXPECT_EQ(insideMenu1, testMenu->getMenu("INSIDEMENU3"));

    EXPECT_EQ(insideMenu1, testMenu->getMenu(0));
    EXPECT_EQ(insideMenu2, testMenu->getMenu(1));
}

TEST(MenuTest, MenuPositionTest) {
    QApplication app(argc, argv);
    ApiMainWindowTest* mainWindow = new ApiMainWindowTest();

    lc::ui::api::Menu* testmenu = mainWindow->addMenu("TESTMENU");

    lc::ui::api::Menu* menu1 = testmenu->addMenu("Menu1");
    lc::ui::api::Menu* menu2 = new lc::ui::api::Menu("Menu2");
    testmenu->addMenu(menu2);
    lc::ui::api::Menu* menu3 = testmenu->addMenu("Menu3");

    EXPECT_EQ(0, menu1->getPosition());
    EXPECT_EQ(1, menu2->getPosition());
    EXPECT_EQ(2, menu3->getPosition());

    menu3->setPosition(0);

    EXPECT_EQ(1, menu1->getPosition());
    EXPECT_EQ(2, menu2->getPosition());
    EXPECT_EQ(0, menu3->getPosition());

    menu2->setPosition(menu3->getPosition());

    EXPECT_EQ(2, menu1->getPosition());
    EXPECT_EQ(0, menu2->getPosition());
    EXPECT_EQ(1, menu3->getPosition());

    menu1->setPosition(2);

    EXPECT_EQ(2, menu1->getPosition());
    EXPECT_EQ(0, menu2->getPosition());
    EXPECT_EQ(1, menu3->getPosition());

    menu3->setPosition(4);

    EXPECT_EQ(1, menu1->getPosition());
    EXPECT_EQ(0, menu2->getPosition());
    EXPECT_EQ(2, menu3->getPosition());

    lc::ui::api::MenuItem* item1 = testmenu->addItem("Item1");

    EXPECT_EQ(1, menu1->getPosition());
    EXPECT_EQ(0, menu2->getPosition());
    EXPECT_EQ(2, menu3->getPosition());
    EXPECT_EQ(3, item1->getPosition());

    item1->setPosition(0);

    EXPECT_EQ(2, menu1->getPosition());
    EXPECT_EQ(1, menu2->getPosition());
    EXPECT_EQ(3, menu3->getPosition());
    EXPECT_EQ(0, item1->getPosition());

    item1->setPosition(2);

    EXPECT_EQ(1, menu1->getPosition());
    EXPECT_EQ(0, menu2->getPosition());
    EXPECT_EQ(3, menu3->getPosition());
    EXPECT_EQ(2, item1->getPosition());
}

TEST(MenuTest, MenuRemoveTest) {
    QApplication app(argc, argv);
    ApiMainWindowTest* mainWindow = new ApiMainWindowTest();

    lc::ui::api::Menu* testmenu = mainWindow->addMenu("TESTMENU");

    lc::ui::api::Menu* menu1 = testmenu->addMenu("Menu1");
    lc::ui::api::Menu* menu2 = new lc::ui::api::Menu("Menu2");
    testmenu->addMenu(menu2);
    lc::ui::api::Menu* menu3 = testmenu->addMenu("Menu3");

    menu3->setPosition(0);
    menu2->setPosition(menu3->getPosition());
    lc::ui::api::MenuItem* item1 = testmenu->addItem("Item1");

    EXPECT_EQ(2, menu1->getPosition());
    EXPECT_EQ(0, menu2->getPosition());
    EXPECT_EQ(1, menu3->getPosition());
    EXPECT_EQ(3, item1->getPosition());

    menu1->remove();

    EXPECT_EQ(0, menu2->getPosition());
    EXPECT_EQ(1, menu3->getPosition());
    EXPECT_EQ(2, item1->getPosition());

    testmenu->removeMenu(menu2);

    EXPECT_EQ(0, menu3->getPosition());
    EXPECT_EQ(1, item1->getPosition());

    item1->setPosition(0);

    EXPECT_EQ(1, menu3->getPosition());
    EXPECT_EQ(0, item1->getPosition());

    testmenu->removeItem("Item1");

    EXPECT_EQ(0, menu3->getPosition());
}

TEST(MenuTest, MenuInBarRemoveTest) {
    QApplication app(argc, argv);
    ApiMainWindowTest* mainWindow = new ApiMainWindowTest();

    lc::ui::api::Menu* menu1 = mainWindow->addMenu("Menu1");
    lc::ui::api::Menu* menu2 = mainWindow->addMenu("Menu2");
    lc::ui::api::Menu* menu3 = new lc::ui::api::Menu("Menu3");
    mainWindow->addMenu(menu3);

    EXPECT_EQ(3, menu1->getPosition());
    EXPECT_EQ(4, menu2->getPosition());
    EXPECT_EQ(5, menu3->getPosition());

    menu3->setPosition(3);

    EXPECT_EQ(4, menu1->getPosition());
    EXPECT_EQ(5, menu2->getPosition());
    EXPECT_EQ(3, menu3->getPosition());

    menu1->remove();

    EXPECT_EQ(4, menu2->getPosition());
    EXPECT_EQ(3, menu3->getPosition());

    lc::ui::api::Menu* menu4 = mainWindow->addMenu("Menu4");

    EXPECT_EQ(4, menu2->getPosition());
    EXPECT_EQ(3, menu3->getPosition());
    EXPECT_EQ(5, menu4->getPosition());

    menu3->remove();

    EXPECT_EQ(3, menu2->getPosition());
    EXPECT_EQ(4, menu4->getPosition());
}
