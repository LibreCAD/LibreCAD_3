#include <lclua.h>
#include <managers/luacustomentitymanager.h>
#include "luainterface.h"

#include <QDir>
#include <QFileDialog>

#include "mainwindow.h"
#include "operationloader.h"

// Phase 4 PR-9a — LuaInterface's event registry delegates to EventBus.
// The Lua-facing overloads wrap kaguya::LuaRef via the Lua adapter.
#include <scriptadapter/luacallback.h>

// Phase 5 PR-5.3 — path.py execution + lcUIPy autoregister.
// Phase 5 PR-5.5 — Python plugin loader called from initLua.
#ifdef LC_WITH_PYTHONSCRIPT
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <lcpython.h>   // PythonInit
#include "python/pypluginloader.h"
#endif

using namespace lc::ui;

LuaInterface::LuaInterface() :
    _pluginManager(_L.state(), "gui") {
}

LuaInterface::~LuaInterface() {
    // Phase 4 PR-9a — was `_events.clear()`; EventBus has its own clear.
    _eventBus.clear();

    lc::lua::LuaCustomEntityManager::getInstance().removePlugins();
}

void LuaInterface::initLua(QMainWindow* mainWindow) {
    auto lcLua = lc::lua::LCLua(_L.state());
    lcLua.setF_openFileDialog(&LuaInterface::openFileDialog);
    lcLua.addLuaLibs();
    lcLua.importLCKernel();

    luaOpenGUIBridge(_L.state());

    _L["luaInterface"] = this;
    registerGlobalFunctions(mainWindow);

    QString luaFile = QCoreApplication::applicationDirPath() + "/path.lua";
    bool s = _L.dofile(luaFile.toStdString().c_str());

    std::string luaPath = _L["lua_path"];
    lc::ui::OperationLoader opLoader(luaPath, mainWindow, _L);

    // Phase 5 PR-5.3 — path bootstrap.  Execute path.py (auto-generated
    // by lcUIPy/CMakeLists.txt at build time; installed next to
    // path.lua in bin/) to insert lcUIPy's parent dir onto sys.path.
    // After this runs, `import lcUIPy.operations` and friends work —
    // OperationLoader::loadPythonOperations (called below) can then
    // discover any modules that autoregister via @lc.register_operation
    // during their own top-level import.
    //
    // Failing gracefully: if path.py doesn't exist (unittest binary
    // in a weird runtime dir, or WITH_PYTHONSCRIPT=OFF) the error text
    // is logged and the Python second source proceeds with whatever
    // registry state exists — matches the "empty registry is OK for
    // PR-5.2" contract.
#ifdef LC_WITH_PYTHONSCRIPT
    {
        QString pyFile =
            QCoreApplication::applicationDirPath() + "/path.py";
        lc::python::PythonInit::initialize();
        pybind11::gil_scoped_acquire gil;
        try {
            pybind11::eval_file(pyFile.toStdString(),
                pybind11::module_::import("__main__").attr("__dict__"));
        } catch (const pybind11::error_already_set& e) {
            // Missing/malformed path.py isn't fatal — log and continue.
            LOG_WARNING << "path.py load failed: " << e.what()
                        << std::endl;
        }

        // Best-effort autoregister: import every module under lcUIPy/
        // that begins with `create_` or `action_` (matches Lua's
        // createActions/actions folder scan).  If import raises, log
        // and continue — a broken module shouldn't prevent the rest.
        try {
            pybind11::exec(R"py(
import importlib, os, sys
try:
    import lcUIPy   # lets sys.path lookup validate first
    _lcuipy_root = os.path.dirname(lcUIPy.__file__)
    for _fname in sorted(os.listdir(_lcuipy_root)):
        if not _fname.endswith('.py'):
            continue
        if not (_fname.startswith('create_') or _fname.startswith('action_')):
            continue
        _mod = 'lcUIPy.' + _fname[:-3]
        try:
            importlib.import_module(_mod)
        except Exception as _e:
            # Log and move on — one broken op shouldn't kill startup.
            print('lcUIPy load: skipping', _mod, ':', _e)
    del _lcuipy_root
except Exception as _e:
    print('lcUIPy autoregister skipped:', _e)
)py",
                pybind11::module_::import("__main__").attr("__dict__"));
        } catch (const pybind11::error_already_set& e) {
            LOG_WARNING << "lcUIPy autoregister failed: "
                        << e.what() << std::endl;
        }
    }
#endif

    if (s) {
        const char* out = lua_tostring(_L.state(), -1);
        if (out == nullptr) {
            LOG_WARNING << "Lua output null" << std::endl;
        }
        else {
            LOG_INFO << "Lua output:" << out << std::endl;
        }
        lua_pop(_L.state(), 1);
    }

    _pluginManager.loadPlugins();

    // Phase 5 PR-5.5 fixup — Python plugin loading MOVED to BEFORE
    // loadPythonOperations.  Rationale (confirmed by coordinator's
    // review of PR-5.5): `@lc.register_operation` in a plugin.py
    // populates `lc.operation_registry` but performs zero wiring;
    // OperationLoader::loadPythonOperations is the ONE-SHOT walk that
    // wires CLI/menu/toolbar entries FROM that registry.  If plugins
    // load AFTER loadPythonOperations, their `@register_operation`
    // classes sit in the registry with no UI hookup — the gear sample
    // literally can't be reached via CLI/menu/toolbar.  Load plugins
    // FIRST so their registrations are visible to the operation walk.
    //
    // Diverges from Lua's exact ordering (Lua plugins load AFTER
    // Lua ops) but Lua plugins use `create_button` / `luaConnect`
    // for their own UI hookup, not the ops registry.  Python's
    // decorator-based registration requires the reverse.
#ifdef LC_WITH_PYTHONSCRIPT
    lc::ui::python::loadPythonPluginsFromPathPy("gui");

    // Phase 5 PR-5.2 — Python second source.  Walks
    // `lc.operation_registry` (populated by @lc.register_operation
    // across lcUIPy/ autoregister AND every plugin.py that ran above)
    // and wires each Python operation through the same
    // CliCommand/Toolbar/Menu/ContextMenu paths.
    opLoader.loadPythonOperations();
#endif
}

QWidget* LuaInterface::loadUiFile(const char* fileName) {
    QUiLoader uiLoader;
    QFile file(fileName);
    file.open(QFile::ReadOnly);

    QWidget* widget = uiLoader.load(&file);

    file.close();

    return widget;
}

void LuaInterface::hideUI(bool hidden) {
    _L["hideUI"] = hidden;
}

lua_State* LuaInterface::luaState() {
    return _L.state();
}

std::vector<std::string> LuaInterface::pluginList(const char* path) {
    std::vector<std::string> plugins;
    QDir dir(path);

    auto list = dir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    for(const auto& dir : list) {
        plugins.push_back(dir.toStdString());
    }

    return plugins;
}

FILE* LuaInterface::openFileDialog(bool isOpening, const char* description, const char* mode) {
    QString path;

    if(isOpening) {
        path = QFileDialog::getOpenFileName(nullptr, (std::string("Open ") + description).c_str());
    }
    else {
        path = QFileDialog::getSaveFileName(nullptr, (std::string("Save ") + description).c_str());
    }

    if(path.isEmpty()) {
        return nullptr;
    }

    return fopen(path.toStdString().c_str(), mode);
}

lc::scripting::ScriptObject LuaInterface::operation() {
    return _operation;
}

void LuaInterface::setOperation(lc::scripting::ScriptObject operation) {
    _operation = std::move(operation);
}

void LuaInterface::finishOperation() {
    // Phase 4 PR-7: was `_operation["close"](_operation)`.  ScriptObject's
    // callMethod prepends `self` automatically for the Lua adapter, so
    // this matches the Lua `op:close()` call shape exactly.
    if (!_operation.isNil() && _operation.hasAttr("close")) {
        _operation.callMethod("close");
    }
}

void LuaInterface::registerEvent(const std::string& event, const kaguya::LuaRef& callback) {
    // Phase 4 PR-9a — reject-object-without-onEvent guard preserved
    // verbatim (was inline in this function; still enforced at the Lua
    // adapter boundary here so behavior is identical to the pre-refactor
    // path).  Then wrap in a ScriptCallback and delegate.
    if (callback.type() == LUA_TTABLE && callback["onEvent"].isNilref()) {
        return;
    }
    _eventBus.registerEvent(event, lc::lua::makeLuaCallback(callback));
}

void LuaInterface::registerEvent(const std::string& event, lc::scripting::ScriptCallback callback) {
    // Native-side entry point.  No LuaRef inspection — the ScriptCallback
    // adapter's own invokeEvent handles the shape decision (Python's
    // hasattr("onEvent") check, Lua's LuaRef type check).
    _eventBus.registerEvent(event, std::move(callback));
}

void LuaInterface::deleteEvent(const std::string& event, const kaguya::LuaRef& callback) {
    // Phase 4 PR-9a — wrap and delegate.  LuaCallbackImpl::equals
    // delegates to LuaRef ==, so wrapping the SAME LuaRef produces
    // equal ScriptCallbacks — matches the pre-refactor find-by-== path.
    _eventBus.deleteEvent(event, lc::lua::makeLuaCallback(callback));
}

bool LuaInterface::deleteEvent(const std::string& event,
                               const lc::scripting::ScriptCallback& callback) {
    // Phase 5 PR-5.1 — native ScriptCallback overload.  Used by the
    // `lc.event.deregister` hook path so Python operations can uninstall
    // their own listeners on close().
    return _eventBus.deleteEvent(event, callback);
}

void LuaInterface::triggerEvent(const std::string& event, kaguya::LuaRef args) {
    // Phase 4 PR-9a + post-review fix — the LuaRef-taking overload's
    // whole point is Lua-to-Lua dispatch where NO conversion is needed.
    // The naive `_eventBus.triggerEvent(event, fromLua(args))` path is
    // LOSSY:
    //   * fromLua's table case coerces integer keys to strings (kaguya's
    //     lua_tolstring path), so an array table `{10,20,30}` becomes
    //     string-keyed `"1"/"2"/"3"` and `args[1]` / `ipairs` break on
    //     the far side;
    //   * any type ScriptValue can't represent (function, generic
    //     userdata, thread) silently drops to Nil.
    // Fix: iterate the EventBus snapshot ourselves, dispatch Lua
    // listeners with the RAW LuaRef payload (matches the pre-refactor
    // behavior at luainterface.cpp:139-143 verbatim), and fall back to
    // ScriptValue for non-Lua listeners (Python et al) — those need
    // the conversion anyway.  Preserves the double-fire order for
    // operationFinished→finishOperation because EventBus's snapshot
    // returns callbacks in insertion order.
    auto snap = _eventBus.snapshot(event);
    // Lazy fromLua conversion — only pay it if a non-Lua listener
    // actually needs it.
    bool convertedArgs = false;
    lc::scripting::ScriptValue convertedValue;
    for (auto& cb : snap) {
        kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(cb);
        if (!luaRef.isNilref()) {
            // Native Lua-side dispatch — no ScriptValue round-trip.
            if (luaRef.type() == LUA_TFUNCTION) {
                luaRef(event, args);
            } else if (luaRef.type() == LUA_TTABLE) {
                luaRef["onEvent"](luaRef, event, args);
            }
        } else {
            // Cross-language listener — fromLua once, invoke via the
            // adapter's own shape-dispatch (Python's hasattr("onEvent")
            // check, native adapter, ...).
            if (!convertedArgs) {
                convertedValue = lc::lua::fromLua(args);
                convertedArgs = true;
            }
            (void) cb.invokeEvent(event, convertedValue);
        }
    }
}

void LuaInterface::triggerEvent(const std::string& event,
                                const lc::scripting::ScriptValue& args) {
    // Phase 4 PR-9a — native-side entry point.  Used by MainWindow's
    // trigger* slots after PR-9b's payload switch: they build
    // ScriptValue payloads directly (Coordinate / Int / Map) and skip
    // the LuaRef round-trip.
    _eventBus.triggerEvent(event, args);
}

std::size_t LuaInterface::listenerCount(const std::string& event) const {
    return _eventBus.listenerCount(event);
}

void LuaInterface::registerGlobalFunctions(QMainWindow* mainWindow) {
    _L["mainWindow"] = static_cast<lc::ui::MainWindow*>(mainWindow);
    // Phase 4 PR-8 — `run_basic_operation` bridge finally removed.  Its
    // last Lua-side callers (ContextMenuManager's 19 dostrings) are gone;
    // ContextMenuManager's Creation/Dimensions/Modify loops now hit the
    // native `MainWindow::runOperationByName` entry point (added PR-7)
    // directly.  External Lua scripts that need to spawn operations
    // should use `mainWindow:runOperationByName(name, initMethod)` too.
    _L.dostring("finish_operation = function() luaInterface:finishOperation() end");
    _L.dostring("operationFinished = function() mainWindow:operationFinished() end");

    // cli command helper functions
    _L.dostring("message = function(m) mainWindow:cliCommand():write(tostring(m)) end");
    _L.dostring("cli_get_text = function(getText) mainWindow:cliCommand():returnText(getText) end");
    _L.dostring("add_command = function(command, callback) mainWindow:cliCommand():addCommand(command, callback) end");
    _L.dostring("run_command = function(command) mainWindow:cliCommand():runCommand(command) end");
    _L.dostring("add_command('CLEAR', function() mainWindow:cliCommand():clear() end)");
    _L.dostring("CreateDialogWidget = function(widgetName) return gui.DialogWidget(widgetName,mainWindow) end");

    _L.dostring("luaInterface:registerEvent('finishOperation', finish_operation)");
    _L.dostring("luaInterface:registerEvent('operationFinished', operationFinished)");
}
