#pragma once

#include <mainwindow.h>
#include <iostream>

using namespace lc::ui;

/* For use by GUI API tests when they require access luaInterface and other mainwindow functions*/
class ApiMainWindowTest : public MainWindow {
public:
    ApiMainWindowTest()
        : MainWindow()
    {}

    lc::ui::LuaInterface* getLuaInterface() {
        return &luaInterface;
    }
};
