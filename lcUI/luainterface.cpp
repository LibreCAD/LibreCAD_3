#include <lclua.h>
#include <managers/luacustomentitymanager.h>
#include "luainterface.h"

#include <QDir>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMetaObject>
#include <QPointer>
#include <QStringList>
#include <algorithm>
#include <exception>
#include <iostream>
#include <vector>

#include "mainwindow.h"
#include "operationloader.h"
#ifdef LC_WITH_PYTHONSCRIPT
#include "python/pyeventhooks.h"
#endif

// Phase 4 PR-9a — LuaInterface's event registry delegates to EventBus.
// The Lua-facing overloads wrap kaguya::LuaRef via the Lua adapter.
#include <scriptadapter/luacallback.h>

using namespace lc::ui;

namespace {
class ScopedBoolValue final {
public:
    ScopedBoolValue(bool& value, bool replacement)
        : _value(value), _previous(value) {
        _value = replacement;
    }

    ~ScopedBoolValue() { _value = _previous; }

    ScopedBoolValue(const ScopedBoolValue&) = delete;
    ScopedBoolValue& operator=(const ScopedBoolValue&) = delete;

private:
    bool& _value;
    bool _previous;
};

QString resolveLuaPackageRoot() {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir(appDir).filePath("lcUILua"),
        QDir(appDir).filePath("../lcUILua"),
        QDir(appDir).filePath("../share/librecad/lcUILua"),
        QDir(appDir).filePath("../Resources/lcUILua"),
        QDir(appDir).filePath("../Resources/librecad/lcUILua"),
    };

    for (const QString& candidate : candidates) {
        const QString root = QDir::cleanPath(candidate);
        if (QFileInfo(QDir(root).filePath("actions/operations.lua")).isFile()) {
            return QFileInfo(root).absoluteFilePath();
        }
    }
    return {};
}

}

LuaInterface::LuaInterface() :
    _pluginManager(_L.state(), "gui") {
}

LuaInterface::~LuaInterface() {
    // Phase 4 PR-9a — was `_events.clear()`; EventBus has its own clear.
    _eventBus.clear();
    _eventBus.setDiagnosticSink({});

    // Phase 6 PR-6.1 — MULTI-WINDOW BUG FIX.  Previously this destructor
    // called `LuaCustomEntityManager::getInstance().removePlugins()`,
    // which was catastrophically wrong: the manager is a process-global
    // SINGLETON, so closing ONE window's LuaInterface would silently
    // clear custom-entity plugins for ALL open windows.  Reopening a
    // DXF file with custom entities in any surviving window would fail
    // to reconstruct them (they'd appear as inert inserts).
    //
    // The fix is process-scoped lifecycle: plugins live for the
    // process, torn down only at the singleton's own destructor at
    // process exit.  Do NOT reintroduce a per-window removePlugins()
    // call here.  If you need per-window plugin scoping, refactor
    // the manager to a refcount-per-owner model — do NOT restore this
    // singleton-wide clear.
    //
    // Regression guard: unittest/python/pythonbindings_test.cpp's
    // `RegisterPluginPersistsInSingleton` documents the intended
    // process-scoped behavior via a headless simulation.
}

void LuaInterface::initLua(QMainWindow* mainWindow) {
    _owner = dynamic_cast<MainWindow*>(mainWindow);
#ifdef LC_WITH_PYTHONSCRIPT
    lc::ui::python::MainWindowExecutionScope ownerScope(_owner.data());
#endif
    if (auto* window = dynamic_cast<MainWindow*>(mainWindow)) {
        const QPointer<widgets::CliCommand> output(window->cliCommand());
        _eventBus.setDiagnosticSink([output](const std::string& message) {
            if (!output) {
                std::cerr << message << '\n';
                return;
            }
            const QString html = QStringLiteral("<pre>") +
                QString::fromStdString(message).toHtmlEscaped() +
                QStringLiteral("</pre>");
            auto* target = output.data();
            const bool queued = QMetaObject::invokeMethod(
                target,
                [output, html]() {
                    if (output) {
                        output->write(html.toUtf8().toStdString());
                    }
                },
                Qt::QueuedConnection);
            if (!queued) {
                std::cerr << message << '\n';
            }
        });
    }

    auto lcLua = lc::lua::LCLua(_L.state());
    lcLua.setF_openFileDialog(&LuaInterface::openFileDialog);
    lcLua.addLuaLibs();
    lcLua.importLCKernel();

    luaOpenGUIBridge(_L.state());

    _L["luaInterface"] = this;
    registerGlobalFunctions(mainWindow);

    QString luaFile = QCoreApplication::applicationDirPath() + "/path.lua";
    const bool pathLoaded = _L.dofile(luaFile.toStdString().c_str());
    if (!pathLoaded) {
        LOG_WARNING << "Lua path bootstrap failed: "
                    << luaFile.toStdString() << std::endl;
    }

    std::string luaPath = pathLoaded ? _L["lua_path"].get<std::string>() : "";
    const QString luaPackageRoot = resolveLuaPackageRoot();
    if (!luaPackageRoot.isEmpty()) {
        luaPath = luaPackageRoot.toStdString();
        _L["lua_path"] = luaPath;
        _L["plugin_path"] = QDir(luaPackageRoot).filePath("plugins").toStdString();
    }
    lc::ui::OperationLoader opLoader(luaPath, mainWindow, _L);

    _pluginManager.loadPlugins();

#ifdef LC_WITH_PYTHONSCRIPT
    // Definitions/hooks are already bootstrapped once without a UI owner.
    // Wire their shared registry independently for this window, including
    // operation init under this LuaInterface's owner scope.
    opLoader.loadPythonOperations();
#endif
}

QWidget* LuaInterface::loadUiFile(const char* fileName) {
    QUiLoader uiLoader;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        return nullptr;
    }

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

QObject* LuaInterface::ownerObject() const {
    return _owner.data();
}

void LuaInterface::setOperation(lc::scripting::ScriptObject operation) {
    _operation = std::move(operation);
}

void LuaInterface::finishOperation() {
    // Phase 4 PR-7: was `_operation["close"](_operation)`.  ScriptObject's
    // callMethod prepends `self` automatically for the Lua adapter, so
    // this matches the Lua `op:close()` call shape exactly.
    #ifdef LC_WITH_PYTHONSCRIPT
    lc::ui::python::MainWindowExecutionScope ownerScope(_owner.data());
    #endif
    // Remove existing operation listeners before close can fire events. Any
    // registrations made by close itself are tracked and removed immediately
    // afterward, even when close throws.
    cancelOperationEventTracking();
    std::string closeError;
    {
        ScopedBoolValue operationTracking(_trackOperationEvents, true);
        try {
            if (!_operation.isNil() && _operation.hasAttr("close")) {
                lc::scripting::ScriptValue ignored;
                (void)_operation.callMethodChecked("close", ignored, closeError);
            }
        } catch (const std::exception& error) {
            closeError = error.what();
        } catch (...) {
            closeError = "operation close failed with an unknown exception";
        }
    }

    // A failed close must not leave operation-owned listeners live. Queue the
    // diagnostic only after exact cleanup has completed.
    cancelOperationEventTracking();
    if (!closeError.empty()) {
        _eventBus.reportDiagnostic("Script operation close failed:\n" +
                                   closeError);
    }
}

bool LuaInterface::callOperationMethodChecked(
    const std::string& method,
    const std::vector<lc::scripting::ScriptValue>& args,
    lc::scripting::ScriptValue& result,
    std::string& error) {
    if (_operation.isNil()) {
        error = "there is no active script operation";
        result = lc::scripting::ScriptValue{};
        return false;
    }
#ifdef LC_WITH_PYTHONSCRIPT
    lc::ui::python::MainWindowExecutionScope ownerScope(_owner.data());
#endif
    ScopedBoolValue operationTracking(_trackOperationEvents, true);
    return _operation.callMethodChecked(method, result, error, args);
}

lc::scripting::ScriptValue LuaInterface::getOperationAttribute(
    const std::string& name) {
    if (_operation.isNil()) {
        return lc::scripting::ScriptValue{};
    }
#ifdef LC_WITH_PYTHONSCRIPT
    lc::ui::python::MainWindowExecutionScope ownerScope(_owner.data());
#endif
    ScopedBoolValue operationTracking(_trackOperationEvents, true);
    return _operation.getAttr(name);
}

bool LuaInterface::setOperationAttributeChecked(
    const std::string& name,
    const lc::scripting::ScriptValue& value,
    std::string& error) {
    if (_operation.isNil()) {
        error = "there is no active script operation";
        return false;
    }
#ifdef LC_WITH_PYTHONSCRIPT
    lc::ui::python::MainWindowExecutionScope ownerScope(_owner.data());
#endif
    ScopedBoolValue operationTracking(_trackOperationEvents, true);
    return _operation.setAttrChecked(name, value, error);
}

void LuaInterface::beginOperationEventTracking() {
    cancelOperationEventTracking();
    _trackOperationEvents = true;
}

void LuaInterface::endOperationEventTracking() {
    _trackOperationEvents = false;
}

void LuaInterface::cancelOperationEventTracking() {
    _trackOperationEvents = false;
    for (const auto& registration : _operationEventRegistrations) {
        _eventBus.deleteRegistration(registration.first, registration.second);
    }
    _operationEventRegistrations.clear();
}

bool LuaInterface::isOperationEventRegistration(
    lc::scripting::EventBus::RegistrationId id) const {
    return std::any_of(
        _operationEventRegistrations.begin(), _operationEventRegistrations.end(),
        [id](const auto& registration) { return registration.second == id; });
}

void LuaInterface::failOperationAfterCallbackFailure() {
    // Drop the failed operation's listeners before invoking close or
    // operationFinished callbacks, preventing a failing operation callback
    // from re-entering during teardown.
    cancelOperationEventTracking();
    triggerEvent("operationFinished", lc::scripting::ScriptValue{});
    if (!_operation.isNil()) {
        finishOperation();
        _operation = lc::scripting::ScriptObject{};
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
    (void)registerEventTracked(event, lc::lua::makeLuaCallback(callback));
}

void LuaInterface::registerEvent(const std::string& event, lc::scripting::ScriptCallback callback) {
    // Native-side entry point.  No LuaRef inspection — the ScriptCallback
    // adapter's own invokeEvent handles the shape decision (Python's
    // hasattr("onEvent") check, Lua's LuaRef type check).
    (void)registerEventTracked(event, std::move(callback));
}

lc::scripting::EventBus::RegistrationId LuaInterface::registerEventTracked(
    const std::string& event, lc::scripting::ScriptCallback callback) {
    const auto id = _eventBus.registerEventTracked(event, std::move(callback));
    if (_trackOperationEvents) {
        _operationEventRegistrations.emplace_back(event, id);
    }
    return id;
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

bool LuaInterface::deleteEvent(
    const std::string& event,
    lc::scripting::EventBus::RegistrationId id) {
    return _eventBus.deleteRegistration(event, id);
}

void LuaInterface::triggerEvent(const std::string& event, kaguya::LuaRef args) {
#ifdef LC_WITH_PYTHONSCRIPT
    lc::ui::python::MainWindowExecutionScope ownerScope(_owner.data());
#endif
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
    // returns entries in insertion order.
    auto snap = _eventBus.snapshotEntries(event);
    const auto diagnostics = _eventBus.diagnosticSink();
    // Lazy fromLua conversion — only pay it if a non-Lua listener
    // actually needs it.
    bool convertedArgs = false;
    lc::scripting::ScriptValue convertedValue;
    std::vector<lc::scripting::EventBus::RegistrationId> failedRegistrations;
    bool operationCallbackFailed = false;
    for (auto& registration : snap) {
        auto& cb = registration.callback;
        const bool operationOwned =
            isOperationEventRegistration(registration.id);
        ScopedBoolValue operationTracking(
            _trackOperationEvents, _trackOperationEvents || operationOwned);
        bool callbackFailed = false;
        const lc::scripting::ScriptDiagnosticSink callbackDiagnostics =
            [&callbackFailed, &diagnostics](const std::string& message) {
                callbackFailed = true;
                lc::scripting::reportScriptDiagnostic(diagnostics, message);
            };
        kaguya::LuaRef luaRef = lc::lua::unwrapLuaCallback(cb);
        if (!luaRef.isNilref()) {
            // Native Lua-side dispatch — no ScriptValue round-trip.
            (void)lc::lua::invokeLuaEventRaw(
                luaRef, event, args, callbackDiagnostics);
        } else {
            // Cross-language listener — fromLua once, invoke via the
            // adapter's own shape-dispatch (Python's hasattr("onEvent")
            // check, native adapter, ...).
            try {
                if (!convertedArgs) {
                    convertedValue = lc::lua::fromLua(args);
                    convertedArgs = true;
                }
                (void) cb.invokeEvent(event, convertedValue, callbackDiagnostics);
            } catch (const std::exception& error) {
                callbackDiagnostics(
                    "Script event '" + event + "' callback (" + cb.runtime() +
                        ") failed: " + error.what());
            } catch (...) {
                callbackDiagnostics(
                    "Script event '" + event + "' callback (" + cb.runtime() +
                        ") failed with an unknown exception");
            }
        }
        if (callbackFailed) {
            if (operationOwned) {
                operationCallbackFailed = true;
            } else {
                failedRegistrations.push_back(registration.id);
            }
        }
    }

    // Apply cleanup after the snapshot finishes so duplicate callbacks retain
    // independent identities and no listener is removed during iteration. A
    // failing operation callback invalidates the whole operation subscription
    // set; an unrelated listener failure removes only its own registration.
    if (operationCallbackFailed) {
        cancelOperationEventTracking();
    }
    for (const auto id : failedRegistrations) {
        _eventBus.deleteRegistration(event, id);
    }
    if (operationCallbackFailed) {
        failOperationAfterCallbackFailure();
    }
}

void LuaInterface::triggerEvent(const std::string& event,
                                const lc::scripting::ScriptValue& args) {
#ifdef LC_WITH_PYTHONSCRIPT
    lc::ui::python::MainWindowExecutionScope ownerScope(_owner.data());
#endif
    // Native-side events retain registration IDs through dispatch so Python
    // operation callbacks can extend their subscriptions and a failing
    // operation callback can invalidate the whole set after the snapshot.
    const auto snapshot = _eventBus.snapshotEntries(event);
    const auto diagnostics = _eventBus.diagnosticSink();
    std::vector<lc::scripting::EventBus::RegistrationId> failedRegistrations;
    bool operationCallbackFailed = false;
    for (const auto& registration : snapshot) {
        const bool operationOwned =
            isOperationEventRegistration(registration.id);
        ScopedBoolValue operationTracking(
            _trackOperationEvents, _trackOperationEvents || operationOwned);
        bool callbackFailed = false;
        const lc::scripting::ScriptDiagnosticSink callbackDiagnostics =
            [&callbackFailed, &diagnostics](const std::string& message) {
                callbackFailed = true;
                lc::scripting::reportScriptDiagnostic(diagnostics, message);
            };
        try {
            (void)registration.callback.invokeEvent(
                event, args, callbackDiagnostics);
        } catch (const std::exception& error) {
            callbackDiagnostics(
                "Script event '" + event + "' callback (" +
                registration.callback.runtime() + ") failed: " + error.what());
        } catch (...) {
            callbackDiagnostics(
                "Script event '" + event + "' callback (" +
                registration.callback.runtime() +
                ") failed with an unknown exception");
        }
        if (callbackFailed) {
            if (operationOwned) {
                operationCallbackFailed = true;
            } else {
                failedRegistrations.push_back(registration.id);
            }
        }
    }

    // Defer removal until all snapshot entries have run, so an operation
    // failure does not suppress unrelated listeners for this same event.
    if (operationCallbackFailed) {
        cancelOperationEventTracking();
    }
    for (const auto id : failedRegistrations) {
        _eventBus.deleteRegistration(event, id);
    }
    if (operationCallbackFailed) {
        failOperationAfterCallbackFailure();
    }
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
