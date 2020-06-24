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
        QList<QWidget*> childWidgets = _toolbar.tabByName("Quick Access")->findChildren<QWidget*>();
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
        auto state = _luaInterface.luaState();
        _luaInterface.triggerEvent("finishOperation", kaguya::LuaRef(state));

        return _toolbar.tabByName("Current operation") == nullptr;
    }

    bool testRunOperation()
    {
        kaguya::State state(_luaInterface.luaState());
        state.dostring("mainWindow:runOperation(LineOperations, '_init_p2')");

        kaguya::LuaRef curOperation = _luaInterface.operation();

        bool checkCorrectOperation = curOperation["command_line"] == "LINE";
        bool checkCurrentOperationGroupAdded = (_toolbar.tabByName("Quick Access")->groupByName("Current operation") != nullptr);

        return checkCorrectOperation && checkCurrentOperationGroupAdded;
    }

    bool testAddOperationOptions()
    {
        std::vector<kaguya::LuaRef> ops;
        ops.push_back(kaguya::LuaRef());

        addOperationOptions("TEST", ops);

        return operation_options.find("TEST") != operation_options.end();
    }

    bool testConnectMenuItem()
    {
        QAction* testAction = findMenuItemByObjectName("action2_Point_Line");

        kaguya::State state(_luaInterface.luaState());
        state.dostring("test_menu = function() testMenuItem=5 end");
        connectMenuItem("action2_Point_Line", state["test_menu"]);

        testAction->trigger();

        kaguya::LuaRef testMenuItem = state["testMenuItem"];
        bool condition1 = !testMenuItem.isNilref();
        bool condition2 = testMenuItem.get<int>() == 5;

        return condition1 && condition2;
    }
};
