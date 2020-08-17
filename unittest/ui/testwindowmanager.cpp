#include<QApplication>

#include <gtest/gtest.h>
#include <windowmanager.h>

#include "uitests.h"

//REMEMBER THAT MAINWINDOWS IS COMMON FOR ALL TESTS, SO KEEP THAT IN MIND

TEST(WindowManagerTest, InitializationTest){
    QApplication app(argc, argv);
    lc::ui::WindowManager::init();
    lc::ui::WindowManager::getMainWindows()[0]->hide();

    EXPECT_EQ(1, lc::ui::WindowManager::getMainWindows().size());

    lc::ui::WindowManager::getMainWindows().clear();
}

TEST(WindowManagerTest, NewFileTest) {
    QApplication app(argc, argv);
    lc::ui::WindowManager::init();

    lc::ui::MainWindow* prevWindow = lc::ui::WindowManager::getMainWindows()[0];
    lc::ui::WindowManager::newFile(prevWindow);

    lc::ui::WindowManager::getMainWindows()[1]->hide();

    EXPECT_EQ(2, lc::ui::WindowManager::getMainWindows().size());
    EXPECT_TRUE(prevWindow == lc::ui::WindowManager::getMainWindows()[0]);
}
