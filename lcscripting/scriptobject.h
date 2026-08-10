#pragma once

// ScriptObject — the "operation instance" object shape.  Phase 4 PR-1
// follow-on.
//
// A Lua table today (LuaInterface::_operation) / a Python object later.
// Backs `LuaInterface::_operation`, `MainWindow::_oldOperation`, and the
// TextDialog usage.  See the phase-4 sub-plan "ScriptObject" bullet for
// the exact per-caller contract.
//
// Shape (from the sub-plan):
//   * `call()` — constructor invocation (mainwindow.cpp:155)
//   * `getAttr/setAttr` — reads/writes named members; primary user is
//     `step` (mainwindow.cpp:163 etc).
//   * `callMethod(name, args)` — calls a named method with a vector of
//     ScriptValue args.  Names used today: `close`, `_init_default` (the
//     FULL name is passed VERBATIM — runOperation uses the string as-is;
//     the `_init_` prefix is the callers' job), `copyEntity`, `isPaused`,
//     `pauseCreation`, `contextMenuOptions(OpaquePtr menu)`.
//   * `hasAttr(name)` — attribute existence check.
//   * `isNil()` — default-constructed / cleared.
//   * `operator==` — for equality checks (some code paths compare
//     ScriptObjects; the Lua binding uses LuaRef ==).
//
// GIL discipline (Python adapter): identical to ScriptCallback — the
// pImpl acquires the GIL in dtor / copy / every operation.  This is the
// permanent-release GIL pattern per plan decision 7.

#include "scriptvalue.h"

#include <memory>
#include <string>
#include <vector>

namespace lc {
namespace scripting {

/// Per-runtime pImpl for ScriptObject.  Adapters live with their runtime
/// (Lua in lcadluascript, Python in lcadpythonscript).
class ScriptObjectImpl {
public:
    virtual ~ScriptObjectImpl() = default;

    /// Invoke as a callable and return the value the underlying callable
    /// produced.  For a Lua table this is `_ref(args)` yielding a LuaRef
    /// that the adapter reads back into ScriptValue via `fromLua`
    /// (best-effort table→Map conversion).  Callers that need the OBJECT
    /// identity of the returned instance (methods, `self` binding, etc.)
    /// should use `instantiate` instead — it wraps the constructor's
    /// return in another ScriptObjectImpl so runtime method dispatch
    /// keeps working.
    virtual ScriptValue call(const std::vector<ScriptValue>& args) = 0;

    /// Invoke as a constructor and return a fresh ScriptObject wrapping
    /// the returned instance.  This is what MainWindow::runOperation uses
    /// to create an operation instance from an operation-class object —
    /// the instance still has method-callable identity (unlike `call`,
    /// which lossy-converts a table to a Map).  Phase 4 PR-7 add.
    virtual std::shared_ptr<ScriptObjectImpl>
    instantiate(const std::vector<ScriptValue>& args) = 0;

    /// Read a named attribute.  Returns Nil for absent (matches Lua's
    /// `t[k]` on a missing key — no distinction between nil and absent).
    virtual ScriptValue getAttr(const std::string& name) = 0;

    /// Write a named attribute.
    virtual void setAttr(const std::string& name, const ScriptValue& value) = 0;

    /// True iff the attribute exists.  For Lua: `type(t[k]) ~= 'nil'`.
    /// For Python: `hasattr(obj, name)`.
    virtual bool hasAttr(const std::string& name) = 0;

    /// Invoke a named method with args.  Returns the method's return
    /// value (Nil for void).  Method names are passed VERBATIM — the
    /// `_init_` prefix used by runOperation is the caller's job.
    virtual ScriptValue callMethod(const std::string& name,
                                   const std::vector<ScriptValue>& args) = 0;

    /// Equality with another impl.  Lua: LuaRef ==.  Python: is OR equal
    /// (same design decision as ScriptCallback).
    virtual bool equals(const ScriptObjectImpl& other) const = 0;

    /// Runtime tag ("lua", "python", "native") for diagnostics + cross-
    /// runtime equality short-circuit.
    virtual const char* runtime() const = 0;
};

class ScriptObject {
public:
    /// Nil default.
    ScriptObject() = default;

    /// Construct from a pImpl.  Ownership transfers.
    explicit ScriptObject(std::shared_ptr<ScriptObjectImpl> impl)
        : _impl(std::move(impl)) {}

    // Copyable (LuaInterface::_operation, MainWindow::_oldOperation both
    // get copy-assigned).  shared_ptr semantics — adapters must be safe
    // under multi-owner (their operations acquire the language runtime's
    // lock inside the adapter).
    ScriptObject(const ScriptObject&) = default;
    ScriptObject& operator=(const ScriptObject&) = default;
    ScriptObject(ScriptObject&&) noexcept = default;
    ScriptObject& operator=(ScriptObject&&) noexcept = default;

    bool isNil() const { return !_impl; }

    ScriptValue call(const std::vector<ScriptValue>& args = {}) const {
        return _impl ? _impl->call(args) : ScriptValue{};
    }

    /// Constructor-invocation returning a fresh, identity-preserved
    /// ScriptObject.  Nil-on-nil.  Phase 4 PR-7 add.
    ScriptObject instantiate(const std::vector<ScriptValue>& args = {}) const {
        return _impl ? ScriptObject(_impl->instantiate(args)) : ScriptObject{};
    }

    ScriptValue getAttr(const std::string& name) const {
        return _impl ? _impl->getAttr(name) : ScriptValue{};
    }

    void setAttr(const std::string& name, const ScriptValue& value) const {
        if (_impl) _impl->setAttr(name, value);
    }

    bool hasAttr(const std::string& name) const {
        return _impl ? _impl->hasAttr(name) : false;
    }

    ScriptValue callMethod(const std::string& name,
                           const std::vector<ScriptValue>& args = {}) const {
        return _impl ? _impl->callMethod(name, args) : ScriptValue{};
    }

    bool operator==(const ScriptObject& other) const {
        if (!_impl && !other._impl) return true;
        if (!_impl || !other._impl) return false;
        if (_impl->runtime() != other._impl->runtime()) return false;
        return _impl->equals(*other._impl);
    }
    bool operator!=(const ScriptObject& other) const { return !(*this == other); }

    const std::shared_ptr<ScriptObjectImpl>& impl() const { return _impl; }

private:
    std::shared_ptr<ScriptObjectImpl> _impl;
};

} // namespace scripting
} // namespace lc
