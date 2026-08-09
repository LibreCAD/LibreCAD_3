#pragma once

// ScriptCallback — a language-neutral, copyable, pImpl-backed callback
// value.  Phase 4 PR-1.
//
// Design (see phase-4 sub-plan "New types"):
//   * `invoke(std::vector<ScriptValue>)` — the generic entry.
//   * `invokeEvent(event_name, EventArgs)` — the two-shape dispatch that
//     Lua's LuaInterface::triggerEvent uses (luainterface.cpp:133-143).
//   * `operator==` for deleteEvent parity: Lua uses `LuaRef` ==; Python
//     uses `a is b OR a.equals(b)` — the `is`-only trap (bound methods
//     are fresh objects per attribute access) is documented in the
//     master plan's design decisions.
//   * `isNil()` and default construction — tests construct null LuaRefs.
//   * `operator=` copyable — Qt::QMap<QString, ScriptCallback> in
//     CliCommand::_commands_cb needs it.
//   * GIL discipline is INSIDE the Python adapter's pImpl: destructor and
//     copy operations acquire the GIL before touching py::object.  Callers
//     (Qt widgets, the CliCommand map, adapter reassignments) run
//     GIL-free per the permanent-release pattern of plan decision 7.
//
// This header declares the class; the pImpl instances live in
// scriptcallback_native.cpp (std::function adapter) and each language's
// bridge library (Python: lcadpythonscript/scriptadapter/;
// Lua: lcadluascript/scriptadapter/).

#include "scriptvalue.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace lc {
namespace scripting {

/// Per-runtime pImpl for ScriptCallback.  Adapters implement this in their
/// own TUs so lcscripting doesn't need to depend on any language runtime.
class ScriptCallbackImpl {
public:
    virtual ~ScriptCallbackImpl() = default;

    /// Invoke the underlying callable with @p args.  Returns the value the
    /// callable produced (or Nil for void).  Adapters catch language-
    /// specific exceptions internally and route diagnostics to the CLI
    /// command output; they never let an exception unwind through Qt.
    virtual ScriptValue invoke(const std::vector<ScriptValue>& args) = 0;

    /// Invoke as an event handler.  Adapter chooses between the two
    /// callable shapes (callable-that-takes-args vs
    /// object-with-onEvent-method) per language.  For Lua this is:
    ///   * function → `f(event, args)`
    ///   * table    → `t:onEvent(event, args)` (which prepends `t` again).
    /// For Python:
    ///   * callable → call
    ///   * object   → obj.onEvent(event, args).
    virtual ScriptValue invokeEvent(const std::string& event,
                                    const ScriptValue& args) = 0;

    /// Equality comparison, adapter-specific.  See the master plan's
    /// bound-method-identity note for Python.
    virtual bool equals(const ScriptCallbackImpl& other) const = 0;

    /// A stable string describing the runtime ("native", "python", "lua")
    /// so callers building error messages can identify origin.
    virtual const char* runtime() const = 0;
};

class ScriptCallback {
public:
    /// Construct a nil callback.
    ScriptCallback() = default;

    /// Construct from a runtime-specific pImpl.  Ownership transfers.
    explicit ScriptCallback(std::shared_ptr<ScriptCallbackImpl> impl)
        : _impl(std::move(impl)) {}

    /// Copyable — QMap<QString, ScriptCallback> requires this.  The pImpl
    /// is shared (thus adapters must be safe under multi-owner semantics —
    /// their operations touching the language state must acquire the GIL
    /// inside the adapter).
    ScriptCallback(const ScriptCallback&) = default;
    ScriptCallback& operator=(const ScriptCallback&) = default;
    ScriptCallback(ScriptCallback&&) noexcept = default;
    ScriptCallback& operator=(ScriptCallback&&) noexcept = default;

    bool isNil() const { return !_impl; }

    /// Generic invoke.  On nil this returns Nil silently (matches
    /// LuaInterface's LuaRef::call behavior on unresolved refs).
    ScriptValue invoke(const std::vector<ScriptValue>& args = {}) const {
        return _impl ? _impl->invoke(args) : ScriptValue{};
    }

    /// Templated sugar for the widget-callback arities (no varargs
    /// gymnastics at call sites — matches the Lua widget-callback shapes
    /// listed in the phase-4 sub-plan).  Wraps args into a std::vector
    /// once so the pImpl doesn't have to overload per arity.
    template <typename... Args>
    ScriptValue call(Args&&... args) const {
        std::vector<ScriptValue> pack{ScriptValue(std::forward<Args>(args))...};
        return invoke(pack);
    }

    /// Event dispatch — see the impl's onEvent contract.
    ScriptValue invokeEvent(const std::string& event,
                            const ScriptValue& args = ScriptValue{}) const {
        return _impl ? _impl->invokeEvent(event, args) : ScriptValue{};
    }

    /// Equality.  Nil == Nil is true.  Nil == non-nil is false.  Two
    /// pImpls of different runtimes compare unequal.  Otherwise deferred
    /// to the pImpl (which knows the language-specific semantics).
    bool operator==(const ScriptCallback& other) const {
        if (!_impl && !other._impl) return true;
        if (!_impl || !other._impl) return false;
        if (_impl->runtime() != other._impl->runtime()) return false;
        return _impl->equals(*other._impl);
    }
    bool operator!=(const ScriptCallback& other) const { return !(*this == other); }

    /// For adapters that need to reach through — e.g. a Python widget
    /// panel that wants to check `runtime()` to decide whether to
    /// materialize its Map with the Python or Lua adapter (see the
    /// DialogWidget::finishCallbacks trick this replaces).
    const std::shared_ptr<ScriptCallbackImpl>& impl() const { return _impl; }

private:
    std::shared_ptr<ScriptCallbackImpl> _impl;
};

// -----------------------------------------------------------------------------
// Native (std::function) adapter — for code-gen replacements and unit
// tests.  Factory declared here, implementation in scriptcallback_native.cpp.
// -----------------------------------------------------------------------------

/// Wrap a std::function that takes ScriptValue args and returns a
/// ScriptValue.  The most permissive shape — useful for code that was
/// dostring codegen and only needs a native lambda.
ScriptCallback nativeCallback(std::function<ScriptValue(const std::vector<ScriptValue>&)> fn);

/// Wrap a no-arg void lambda (the common case for menu items).
ScriptCallback nativeCallback(std::function<void()> fn);

} // namespace scripting
} // namespace lc
