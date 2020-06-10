#include<QApplication>

#include <gtest/gtest.h>
#include "mainwindowtest.h"

#include "uitests.h"

TEST(MWindowTest, WindowWidgetTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testAddingOfWidgets());
    EXPECT_TRUE(mainWindow->testAddingOfSelectTools());
}

TEST(MWindowTest, PointTriggerTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    // test if last point is updated
    EXPECT_TRUE(mainWindow->testTriggerPoint(lc::geo::Coordinate(2,2)));
}

TEST(MWindowTest, OperationFinishedTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testOperationFinished());
}

TEST(MWindowTest, RunOperationTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testRunOperation());
}

TEST(MWindowTest, AddOperationOptionsTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testAddOperationOptions());
}

TEST(MWindowTest, ConnectMenuTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testConnectMenuItem());
}

TEST(MWindowTest, MenuAPITest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    lc::ui::api::Menu* menu1 = mainWindow->addMenu("Menu1");
    lc::ui::api::Menu* menu2 = new lc::ui::api::Menu("Menu2");
    mainWindow->addMenu(menu2);

    EXPECT_EQ(menu1, mainWindow->getMenu("Menu1"));
    EXPECT_EQ(menu1, mainWindow->getMenu(3));

    EXPECT_EQ(menu2, mainWindow->getMenu("Menu2"));
    EXPECT_EQ(menu2, mainWindow->getMenu(4));

    EXPECT_EQ(3, menu1->position());
    EXPECT_EQ(4, menu2->position());

    mainWindow->removeMenu(0);

    EXPECT_EQ(2, menu1->position());
    EXPECT_EQ(3, menu2->position());

    mainWindow->removeMenu("Create");

    EXPECT_EQ(1, menu1->position());
    EXPECT_EQ(2, menu2->position());
}
