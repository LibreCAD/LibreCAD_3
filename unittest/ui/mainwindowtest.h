#pragma once

#include <mainwindow.h>
#include <iostream>

#include <QSignalSpy>

// Phase 4 PR-4 — connectMenuItem takes ScriptCallback; tests wrap
// LuaRefs via the Lua adapter.
#include <scriptadapter/luacallback.h>

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

    // Phase 4 post-review fix — verify LuaInterface::triggerEvent(LuaRef)
    // passes an array-table payload through to a Lua listener WITHOUT
    // the lossy fromLua/toLua round-trip.  The naive round-trip would
    // coerce integer keys `[1]/[2]/[3]` to string keys `"1"/"2"/"3"`
    // and any Lua function reference to nil.  Regression test for the
    // triggerEvent pass-through path.
    bool testTriggerEventArrayPayloadPreserved()
    {
        kaguya::State state(_luaInterface.luaState());
        // Set up: a Lua-side receiver that records what it received.
        state.dostring(R"(
            _lc_test_array_result = {
                len_ipairs = 0,
                sum_ipairs = 0,
                first_int_key_value = nil,
                fn_type = nil,
            }
            _lc_test_fn = function(event, args)
                for _, v in ipairs(args.arr) do
                    _lc_test_array_result.len_ipairs = _lc_test_array_result.len_ipairs + 1
                    _lc_test_array_result.sum_ipairs = _lc_test_array_result.sum_ipairs + v
                end
                _lc_test_array_result.first_int_key_value = args.arr[1]
                _lc_test_array_result.fn_type = type(args.callback)
            end
        )");
        // .get<LuaRef>() disambiguates: the kaguya proxy converts to both
        // kaguya::LuaRef and lc::scripting::ScriptCallback, and this test
        // exercises the Lua pass-through overload.
        _luaInterface.registerEvent("_lc_test_array_event",
                                    state["_lc_test_fn"].get<kaguya::LuaRef>());

        // Build a payload that HAS to go through the pass-through path
        // to survive: an array table + a function reference.  The
        // pre-fix round-trip would break BOTH — array becomes
        // string-keyed and the function becomes nil.
        state.dostring(R"(
            _lc_test_payload = {
                arr = {10, 20, 30},
                callback = function() return "unused" end,
            }
        )");
        _luaInterface.triggerEvent("_lc_test_array_event",
                                   state["_lc_test_payload"].get<kaguya::LuaRef>());

        // Verify: ipairs iterated 3 entries, summed to 60, args.arr[1]
        // = 10 (integer key resolved), and args.callback is still a
        // function (not nil).
        int len = state["_lc_test_array_result"]["len_ipairs"];
        int sum = state["_lc_test_array_result"]["sum_ipairs"];
        int first = state["_lc_test_array_result"]["first_int_key_value"];
        std::string fnType =
            state["_lc_test_array_result"]["fn_type"].get<std::string>();

        // Cleanup so subsequent runs don't accumulate.
        _luaInterface.deleteEvent("_lc_test_array_event",
                                  state["_lc_test_fn"].get<kaguya::LuaRef>());
        state["_lc_test_payload"]      = nullptr;
        state["_lc_test_fn"]           = nullptr;
        state["_lc_test_array_result"] = nullptr;

        return len == 3 && sum == 60 && first == 10 && fnType == "function";
    }

    bool testRunOperation()
    {
        // Phase 4 PR-7 — exercise the two entrypoints separately:
        //   * The Lua-side `mainWindow:runOperation(cls, init)` path
        //     (guibridge wraps LuaRef→ScriptObject via makeLuaObject).
        //   * The native `runOperationByName(name, init)` path added
        //     in this PR.  Both must produce the same operation-instance
        //     command_line, since they route through the same C++
        //     runOperation body.
        kaguya::State state(_luaInterface.luaState());
        state.dostring("mainWindow:runOperation(LineOperations, '_init_p2')");

        lc::scripting::ScriptObject curOperation = _luaInterface.operation();
        bool checkCorrectOperation =
            curOperation.getAttr("command_line").asString() == "LINE";
        bool checkCurrentOperationGroupAdded =
            (_toolbar.tabByName("Quick Access")->groupByName("Current operation") != nullptr);

        // Native entrypoint round-trip: resolve by name via the ordered
        // resolver list.  Same expectation.
        runOperationByName("LineOperations", "_init_p2");
        lc::scripting::ScriptObject reResolved = _luaInterface.operation();
        bool nativeEntryMatches =
            reResolved.getAttr("command_line").asString() == "LINE";

        return checkCorrectOperation
            && checkCurrentOperationGroupAdded
            && nativeEntryMatches;
    }

    bool testAddOperationOptions()
    {
        // Phase 4 PR-7 — options are neutral ScriptCallbacks.
        std::vector<lc::scripting::ScriptCallback> ops;
        ops.push_back(lc::scripting::ScriptCallback{});

        addOperationOptions("TEST", ops);

        return operation_options.find("TEST") != operation_options.end();
    }

    bool testConnectMenuItem()
    {
        QAction* testAction = findMenuItemByObjectName("action2_Point_Line");

        kaguya::State state(_luaInterface.luaState());
        state.dostring("test_menu = function() testMenuItem=5 end");
        // Phase 4 PR-4 — connectMenuItem takes ScriptCallback; wrap
        // the Lua callable via the adapter.  Behavior unchanged: the
        // Lua function still fires when the menu item is triggered.
        connectMenuItem("action2_Point_Line",
                        lc::lua::makeLuaCallback(state["test_menu"]));

        testAction->trigger();

        kaguya::LuaRef testMenuItem = state["testMenuItem"];
        bool condition1 = !testMenuItem.isNilref();
        bool condition2 = testMenuItem.get<int>() == 5;

        return condition1 && condition2;
    }
};
