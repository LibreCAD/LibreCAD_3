#pragma once

#include <mainwindow.h>
#include <iostream>

#include <QSignalSpy>

using namespace lc::ui;

class MainWindowTest : public MainWindow {
public:
    MainWindowTest()
        : MainWindow()
    {}

    bool testAddingOfWidgets()
    {
        QList<QWidget*> childWidgets = findChildren<QWidget*>();
        bool cadMdiChildFound = false;
        bool layersFound = false;
        bool cliCommandFound = false;
        bool toolbarFound = false;

        for (QWidget* childWidget : childWidgets)
        {
            if (dynamic_cast<CadMdiChild*>(childWidget) != nullptr) {
                cadMdiChildFound = true;
            }

            if (dynamic_cast<widgets::Layers*>(childWidget) != nullptr) {
                layersFound = true;
            }

            if (dynamic_cast<widgets::CliCommand*>(childWidget) != nullptr) {
                cliCommandFound = true;
            }

            if (dynamic_cast<widgets::Toolbar*>(childWidget) != nullptr) {
                toolbarFound = true;
            }
        }

        return cadMdiChildFound && layersFound && cliCommandFound && toolbarFound;
    }

    bool testAddingOfSelectTools()
    {
        QList<QWidget*> childWidgets = toolbar.tabByName("Quick Access")->findChildren<QWidget*>();
        bool linePatternSelectFound = false;
        bool lineWidthSelectFound = false;
        bool colorSelectFound = false;

        for (QWidget* childWidget : childWidgets)
        {
            if (dynamic_cast<widgets::LinePatternSelect*>(childWidget) != nullptr) {
                linePatternSelectFound = true;
            }

            if (dynamic_cast<widgets::LineWidthSelect*>(childWidget) != nullptr) {
                lineWidthSelectFound = true;
            }

            if (dynamic_cast<widgets::ColorSelect*>(childWidget) != nullptr) {
                colorSelectFound = true;
            }
        }

        return linePatternSelectFound && lineWidthSelectFound && colorSelectFound;
    }

    bool testTriggerPoint(lc::geo::Coordinate pt)
    {
        emit point(pt);
        return lastPoint == pt;
    }

    bool testOperationFinished()
    {
        auto state = luaInterface.luaState();
        luaInterface.triggerEvent("finishOperation", kaguya::LuaRef(state));

        return toolbar.tabByName("Current operation") == nullptr;
    }
};
