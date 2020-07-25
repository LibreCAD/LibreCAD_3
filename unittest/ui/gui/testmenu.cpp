#include<QApplication>
#include <gtest/gtest.h>

#include <mainwindow.h>

#include "../uitests.h"

TEST(MenuTest, MenuLabelTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::api::Menu* testMenu = mainWindow->addMenu("TESTMENU");
    lc::ui::api::Menu* testMenu2 = new lc::ui::api::Menu("TESTMENU2");

    mainWindow->addMenu(testMenu2);

    lc::ui::api::Menu* insideMenu1 = testMenu->addMenu("INSIDEMENU1");

    EXPECT_EQ("TESTMENU", testMenu->label());
    EXPECT_EQ("INSIDEMENU1", insideMenu1->label());

    lc::ui::api::Menu* insideMenu2 = new lc::ui::api::Menu("INSIDEMENU2");
    testMenu->addMenu(insideMenu2);

    EXPECT_EQ("INSIDEMENU2", insideMenu2->label());

    insideMenu2->setLabel("NEWINSIDEMENU2");

    EXPECT_EQ("NEWINSIDEMENU2", insideMenu2->label());
    
    EXPECT_EQ(insideMenu2, testMenu->menuByName("NEWINSIDEMENU2"));
    EXPECT_EQ(nullptr,testMenu->menuByName("INSIDEMENU2"));

    EXPECT_EQ(mainWindow->menuByName("TESTMENU2")->label(), "TESTMENU2");
}

TEST(MenuTest, MenuGetTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::api::Menu* testMenu = mainWindow->addMenu("TESTMENU");
    lc::ui::api::Menu* testMenu2 = new lc::ui::api::Menu("TESTMENU2");

    mainWindow->addMenu(testMenu2);

    lc::ui::api::Menu* insideMenu1 = testMenu->addMenu("INSIDEMENU1");
    lc::ui::api::Menu* insideMenu2 = new lc::ui::api::Menu("INSIDEMENU2");
    testMenu->addMenu(insideMenu2);

    EXPECT_EQ(testMenu, mainWindow->menuByName("TESTMENU"));
    EXPECT_EQ(testMenu2, mainWindow->menuByName("TESTMENU2"));

    EXPECT_EQ(insideMenu1, testMenu->menuByName("INSIDEMENU1"));
    EXPECT_EQ(insideMenu2, testMenu->menuByName("INSIDEMENU2"));

    insideMenu1->setLabel("INSIDEMENU3");

    EXPECT_EQ(nullptr, testMenu->menuByName("INSIDEMENU1"));
    EXPECT_EQ(insideMenu1, testMenu->menuByName("INSIDEMENU3"));

    EXPECT_EQ(insideMenu1, testMenu->menuByPosition(0));
    EXPECT_EQ(insideMenu2, testMenu->menuByPosition(1));
}

TEST(MenuTest, MenuPositionTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::api::Menu* testmenu = mainWindow->addMenu("TESTMENU");

    lc::ui::api::Menu* menu1 = testmenu->addMenu("Menu1");
    lc::ui::api::Menu* menu2 = new lc::ui::api::Menu("Menu2");
    testmenu->addMenu(menu2);
    lc::ui::api::Menu* menu3 = testmenu->addMenu("Menu3");

    EXPECT_EQ(0, menu1->position());
    EXPECT_EQ(1, menu2->position());
    EXPECT_EQ(2, menu3->position());

    menu3->setPosition(0);

    EXPECT_EQ(1, menu1->position());
    EXPECT_EQ(2, menu2->position());
    EXPECT_EQ(0, menu3->position());

    menu2->setPosition(menu3->position());

    EXPECT_EQ(2, menu1->position());
    EXPECT_EQ(0, menu2->position());
    EXPECT_EQ(1, menu3->position());

    menu1->setPosition(2);

    EXPECT_EQ(2, menu1->position());
    EXPECT_EQ(0, menu2->position());
    EXPECT_EQ(1, menu3->position());

    menu3->setPosition(4);

    EXPECT_EQ(1, menu1->position());
    EXPECT_EQ(0, menu2->position());
    EXPECT_EQ(2, menu3->position());

    lc::ui::api::MenuItem* item1 = testmenu->addItem("Item1");

    EXPECT_EQ(1, menu1->position());
    EXPECT_EQ(0, menu2->position());
    EXPECT_EQ(2, menu3->position());
    EXPECT_EQ(3, item1->position());

    item1->setPosition(0);

    EXPECT_EQ(2, menu1->position());
    EXPECT_EQ(1, menu2->position());
    EXPECT_EQ(3, menu3->position());
    EXPECT_EQ(0, item1->position());

    item1->setPosition(2);

    EXPECT_EQ(1, menu1->position());
    EXPECT_EQ(0, menu2->position());
    EXPECT_EQ(3, menu3->position());
    EXPECT_EQ(2, item1->position());
}

TEST(MenuTest, MenuRemoveTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::api::Menu* testmenu = mainWindow->addMenu("TESTMENU");

    lc::ui::api::Menu* menu1 = testmenu->addMenu("Menu1");
    lc::ui::api::Menu* menu2 = new lc::ui::api::Menu("Menu2");
    testmenu->addMenu(menu2);
    lc::ui::api::Menu* menu3 = testmenu->addMenu("Menu3");

    menu3->setPosition(0);
    menu2->setPosition(menu3->position());
    lc::ui::api::MenuItem* item1 = testmenu->addItem("Item1");

    EXPECT_EQ(2, menu1->position());
    EXPECT_EQ(0, menu2->position());
    EXPECT_EQ(1, menu3->position());
    EXPECT_EQ(3, item1->position());

    menu1->remove();

    EXPECT_EQ(0, menu2->position());
    EXPECT_EQ(1, menu3->position());
    EXPECT_EQ(2, item1->position());

    testmenu->removeMenu(menu2);

    EXPECT_EQ(0, menu3->position());
    EXPECT_EQ(1, item1->position());

    item1->setPosition(0);

    EXPECT_EQ(1, menu3->position());
    EXPECT_EQ(0, item1->position());

    testmenu->removeItem("Item1");

    EXPECT_EQ(0, menu3->position());
}

TEST(MenuTest, MenuInBarRemoveTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::api::Menu* menu1 = mainWindow->addMenu("Menu1");
    lc::ui::api::Menu* menu2 = mainWindow->addMenu("Menu2");
    lc::ui::api::Menu* menu3 = new lc::ui::api::Menu("Menu3");
    mainWindow->addMenu(menu3);

    EXPECT_EQ(5, menu1->position());
    EXPECT_EQ(6, menu2->position());
    EXPECT_EQ(7, menu3->position());

    menu3->setPosition(4);

    EXPECT_EQ(6, menu1->position());
    EXPECT_EQ(7, menu2->position());
    EXPECT_EQ(4, menu3->position());

    menu1->remove();

    EXPECT_EQ(6, menu2->position());
    EXPECT_EQ(4, menu3->position());

    lc::ui::api::Menu* menu4 = mainWindow->addMenu("Menu4");

    EXPECT_EQ(6, menu2->position());
    EXPECT_EQ(4, menu3->position());
    EXPECT_EQ(7, menu4->position());

    menu3->remove();

    EXPECT_EQ(5, menu2->position());
    EXPECT_EQ(6, menu4->position());
}
