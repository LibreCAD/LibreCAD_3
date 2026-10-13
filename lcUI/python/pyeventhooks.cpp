// Phase 5 PR-5.1 — install lc.event.register/deregister hooks that
// route to the active MainWindow's LuaInterface EventBus.

#ifdef LC_WITH_PYTHONSCRIPT

#include "pyeventhooks.h"
#include "qtpythonlifetime.h"

#include <qt_keywords_push.h>
#include <pybind11/pybind11.h>
#include <qt_keywords_pop.h>

#include <bridge/py_lc.h>          // Phase 6 PR-6.1 — registerPluginHook
#include <bridge/py_lc_event.h>
#include <scriptadapter/pythoncallback.h>

// Phase 6 PR-6.1 — lc.register_plugin(...) needs the neutralized
// LuaCustomEntityManager (which now accepts ScriptCallback).  Include
// path traverses lcadluascript because lcUI already links against it.
#include <managers/luacustomentitymanager.h>

#include "mainwindow.h"
#include "luainterface.h"
#include "cadmdichild.h"
#include "widgets/guiAPI/menu.h"
#include "lua/opaquetags.h"

#include <QCoreApplication>
#include <QAction>
#include <QMenu>
#include <QPointer>
#include <QThread>

#include <cstring>
#include <stdexcept>
#include <string>
#include <set>

namespace py = pybind11;

namespace lc {
namespace ui {
namespace python {

namespace {

QPointer<lc::ui::MainWindow>& scopedMainWindow() {
    static thread_local QPointer<lc::ui::MainWindow> window;
    return window;
}

MainWindow* receiverWindow(QObject* receiver) {
    const auto parentWindow = [](QObject* object) -> MainWindow* {
        for (; object; object = object->parent())
            if (auto* window = qobject_cast<MainWindow*>(object)) return window;
        return nullptr;
    };
    if (auto* window = parentWindow(receiver)) return window;
    MainWindow* result = nullptr;
    // addAction/addMenu need not transfer parenting. Follow their graph without
    // cycling through QMenu/menuAction or choosing an arbitrary shared window.
    std::vector<QObject*> pending{receiver};
    std::set<QObject*> visited;
    for (std::size_t i = 0; i < pending.size(); ++i) {
        auto* object = pending[i];
        if (!object || !visited.insert(object).second) continue;
        if (auto* window = parentWindow(object)) {
            if (result && result != window)
                throw std::runtime_error("Python GUI callback has ambiguous associated windows");
            result = window;
            continue;
        }
        pending.push_back(object->parent());
        if (auto* menu = qobject_cast<QMenu*>(object)) pending.push_back(menu->menuAction());
        if (auto* action = qobject_cast<QAction*>(object))
            for (auto* associated : action->associatedObjects()) pending.push_back(associated);
    }
    return result;
}

class WindowScopedCallback final : public lc::scripting::ScriptCallbackImpl {
public:
    WindowScopedCallback(lc::scripting::ScriptCallback callback, QObject* receiver)
        : _callback(std::move(callback)), _receiver(receiver),
          _owner(receiverWindow(receiver)) {
        if (!_owner) _owner = currentMainWindow();
        _bound = !_owner.isNull();
    }

    const char* runtime() const override { return _callback.runtime(); }
    bool equals(const lc::scripting::ScriptCallbackImpl& other) const override {
        const auto* scoped = dynamic_cast<const WindowScopedCallback*>(&other);
        return scoped && _receiver == scoped->_receiver && _owner == scoped->_owner &&
            _bound == scoped->_bound && _callback == scoped->_callback;
    }
    lc::scripting::ScriptValue invoke(
        const std::vector<lc::scripting::ScriptValue>& args) override {
        return inScope([&] { return _callback.invoke(args); });
    }
    lc::scripting::ScriptValue invokeEvent(const std::string& event,
        const lc::scripting::ScriptValue& args,
        const lc::scripting::ScriptDiagnosticSink& diagnostics) override {
        return inScope([&] { return _callback.invokeEvent(event, args, diagnostics); }, diagnostics);
    }
    void bindReceiverWindow(QObject* receiver) {
        if (_bound) return;
        _owner = receiverWindow(receiver);
        _bound = !_owner.isNull();
    }

private:
    template<class Call> lc::scripting::ScriptValue inScope(Call call,
        const lc::scripting::ScriptDiagnosticSink& diagnostics = {}) {
        try {
            QtUiThreadCallGuard thread;
            if (_bound && !_owner) return {};
            if (!_bound) {
                if (!_receiver) return {};
                bindReceiverWindow(_receiver);
            }
            MainWindowExecutionScope scope(_owner.data());
            return call();
        } catch (const std::exception& error) {
            lc::scripting::reportScriptDiagnostic(diagnostics, error.what());
            return {};
        }
    }
    lc::scripting::ScriptCallback _callback;
    QPointer<QObject> _receiver;
    QPointer<MainWindow> _owner;
    bool _bound;
};

py::object materializeUiOpaque(const lc::scripting::OpaquePtr& opaque) {
    if (opaque.ptr == nullptr) {
        return py::none();
    }
    if (opaque.tag == nullptr) {
        throw std::runtime_error(
            "Python callback received an opaque UI value without a type tag");
    }

    QCoreApplication* app = QCoreApplication::instance();
    if (app == nullptr || QThread::currentThread() != app->thread()) {
        throw std::runtime_error(
            "Python UI event values must be materialized on the Qt "
            "application thread");
    }

    if (std::strcmp(opaque.tag, lc::ui::opaquetag::CadMdiChild) == 0) {
        py::module_::import("lcgui");
        return py::cast(static_cast<lc::ui::CadMdiChild*>(opaque.ptr),
                        py::return_value_policy::reference);
    }
    if (std::strcmp(opaque.tag, lc::ui::opaquetag::Menu) == 0) {
        py::module_::import("lcgui");
        return py::cast(static_cast<lc::ui::api::Menu*>(opaque.ptr),
                        py::return_value_policy::reference);
    }

    throw std::runtime_error(
        std::string("Python callback cannot materialize opaque UI type '") +
        opaque.tag + "'");
}

} // namespace

MainWindowExecutionScope::MainWindowExecutionScope(MainWindow* window)
    : _previous(scopedMainWindow()) {
    scopedMainWindow() = window;
}

MainWindowExecutionScope::~MainWindowExecutionScope() {
    scopedMainWindow() = _previous;
}

lc::ui::MainWindow* currentMainWindow() {
    return scopedMainWindow().data();
}

lc::scripting::ScriptCallback scopedUiCallback(
    lc::scripting::ScriptCallback callback, QObject* receiver) {
    QtUiThreadCallGuard thread;
    if (!receiver) throw std::runtime_error("Python GUI callback requires a Qt receiver");
    auto scoped = std::make_shared<WindowScopedCallback>(std::move(callback), receiver);
    // Clones share the proxy. Bind a late-adopted original's parent before it
    // disappears, even if no callback has run yet; never retain the receiver.
    QObject::connect(receiver, &QObject::destroyed,
        [weak = std::weak_ptr<WindowScopedCallback>(scoped)](QObject* dying) {
            try {
                QtUiThreadCallGuard thread;
                if (auto callback = weak.lock()) callback->bindReceiverWindow(dying);
            } catch (const std::exception& error) {
                lc::scripting::reportScriptDiagnostic({}, error.what());
            }
        });
    return lc::scripting::ScriptCallback(std::move(scoped));
}

void installEventHooks() {
    lc::python::setPythonOpaqueMaterializer(materializeUiOpaque);

    // Register hook: return the EventBus registration ID so Python owners can
    // dispose one exact entry even when equal callbacks were registered twice.
    lc::python::setEventRegisterHook(
        [](const std::string& name, py::object obj)
            -> lc::python::EventRegistrationId {
            lc::ui::MainWindow* mw = currentMainWindow();
            if (mw == nullptr) return 0;
            return mw->luaInterface()->registerEventTracked(
                name, lc::python::makePythonCallback(std::move(obj)));
        });

    // Deregister hook: same routing, delete instead of register.
    // Phase-4 note on equality: PythonCallbackImpl::equals uses
    // is-or-equal so a fresh `self.on_point` bound-method access still
    // matches the registered one — this is what makes
    // `lc.event.register("point", self.on_point)` /
    // `lc.event.deregister("point", self.on_point)` work even though
    // Python creates a fresh bound-method object per attribute access.
    lc::python::setEventDeregisterHook(
        [](const std::string& name, py::object obj) {
            lc::ui::MainWindow* mw = currentMainWindow();
            if (mw == nullptr) return;
            mw->luaInterface()->deleteEvent(
                name, lc::python::makePythonCallback(std::move(obj)));
        });

    lc::python::setEventRegistrationDeregisterHook(
        [](const std::string& name, lc::python::EventRegistrationId id) {
            lc::ui::MainWindow* mw = currentMainWindow();
            if (mw == nullptr) return;
            mw->luaInterface()->deleteEvent(name, id);
        });

    // Phase 6 PR-6.1 — `lc.register_plugin(name, fn)` hook.  Routes into
    // the process-global LuaCustomEntityManager (neutralized to accept
    // ScriptCallback), so Python-authored plugins share the same
    // manager as Lua-authored ones.  Unlike the event hooks above,
    // there is NO per-MainWindow routing here — the plugin manager is
    // process-scoped by design (see the multi-window bug fix note in
    // luainterface.cpp:31-44).  A Python plugin registered in one
    // window remains registered when that window closes.
    lc::python::setRegisterPluginHook(
        [](const std::string& name, py::object obj) {
            lc::lua::LuaCustomEntityManager::getInstance().registerPlugin(
                name, lc::python::makePythonCallback(std::move(obj)));
        });
}

} // namespace python
} // namespace ui
} // namespace lc

#endif  // LC_WITH_PYTHONSCRIPT
