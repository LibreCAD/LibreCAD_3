#pragma once

// EventBus — event registry + dispatch, extracted from the Lua-side
// `LuaInterface`.  Phase 4 PR-1 follow-on.
//
// Public API (mirrors the phase-4 sub-plan's "New types" bullet):
//   * registerEvent(name, ScriptCallback)  — add a callback for the
//     event `name`.  Duplicate registration is allowed (Lua does the
//     same); deregistration finds the first match by ==.
//   * deleteEvent(name, ScriptCallback)    — remove the first callback
//     whose `operator==` matches.  Silent no-op on miss.
//   * triggerEvent(name, args)             — fire all registered
//     callbacks for `name` with the given args (may be Nil).
//
// Verbatim-preserved quirks (from the sub-plan):
//   * copy-before-dispatch (luainterface.cpp:134): while iterating,
//     callbacks may register/deregister; copy the list first.
//   * reject-object-without-onEvent guard (:118): the ScriptCallback
//     adapter handles this at invoke time (it's already the callback's
//     runtime's decision — Python's PythonCallbackImpl::invokeEvent
//     checks hasattr("onEvent")).
//
// LAZY payload conversion (per-runtime listener counts) is a phase 4/9
// concern — EventBus keeps a total listener count per event; runtimes
// on the trigger path can query it to skip payload materialization.
// (For PR-1 the count is just a size(); a real per-runtime tally lands
// in the phase 4 sub-PR that introduces mouseMove firing.)

#include "scriptcallback.h"
#include "scriptvalue.h"

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace lc {
namespace scripting {

class EventBus {
public:
    using RegistrationId = std::uint64_t;

    struct Registration {
        RegistrationId id;
        ScriptCallback callback;
    };

    EventBus() = default;

    /// Register @p cb as a listener for event @p name.  Same callback
    /// registered twice is stored twice — matches Lua behavior.
    void registerEvent(const std::string& name, ScriptCallback cb);

    /// Register a callback and return an identity for exact ownership and
    /// cleanup. Equal callbacks registered twice receive distinct IDs.
    RegistrationId registerEventTracked(const std::string& name,
                                        ScriptCallback cb);

    /// Remove the first callback whose operator== matches @p cb.
    /// Returns true if a callback was removed.
    bool deleteEvent(const std::string& name, const ScriptCallback& cb);

    /// Remove exactly one registration, even when equal callbacks exist.
    bool deleteRegistration(const std::string& name, RegistrationId id);

    /// Fire the event: dispatch @p args to every registered callback.
    /// Callbacks may register/deregister during dispatch — we copy the
    /// list first (verbatim behavior from luainterface.cpp:134).
    /// Return value from callbacks is discarded (matches Lua's
    /// `pcall(cb, ...)` shape).
    void triggerEvent(const std::string& name,
                      const ScriptValue& args = ScriptValue{});

    /// Set the per-window failure sink. It is copied while holding the bus
    /// mutex and invoked only after the mutex is released.
    void setDiagnosticSink(ScriptDiagnosticSink sink);

    /// Copy the current sink for adapters that perform a raw-runtime dispatch.
    ScriptDiagnosticSink diagnosticSink() const;

    /// Route a raw-dispatch failure through this bus's sink.
    void reportDiagnostic(const std::string& message) const;

    /// Number of callbacks registered for @p name.  Callers use this to
    /// skip lazy payload conversion when no listeners are present
    /// (mouseMove fires per pixel; converting for zero listeners is
    /// waste).
    std::size_t listenerCount(const std::string& name) const;

    /// Number of DISTINCT event names that have any registrations.
    /// Diagnostic only — the LuaInterface tests use this to assert
    /// deregister-on-shutdown left an empty bus.
    std::size_t eventCount() const;

    /// Return a snapshot copy of the callbacks registered for @p name.
    /// Empty vector for unregistered events.  This is the low-level hook
    /// that LuaInterface's LuaRef-taking triggerEvent overload uses to
    /// PASS-THROUGH a raw LuaRef payload to Lua-side listeners without
    /// the lossy fromLua/toLua round-trip.  Same copy-before-dispatch
    /// discipline as triggerEvent (fetch under the mutex, iterate after
    /// release).  Callers are responsible for firing the callbacks
    /// themselves.
    std::vector<ScriptCallback> snapshot(const std::string& name) const;

    /// ID-preserving variant for dispatchers that must clean up exact entries.
    std::vector<Registration> snapshotEntries(const std::string& name) const;

    /// Remove all registrations.  Used at MainWindow teardown.
    void clear();

private:
    // Mutex protects _events.  triggerEvent copies the vector while
    // holding the lock, then releases so callbacks can re-enter for
    // register/deregister.
    mutable std::mutex _mu;
    std::unordered_map<std::string, std::vector<Registration>> _events;
    RegistrationId _nextRegistrationId = 1;
    ScriptDiagnosticSink _diagnosticSink;
};

} // namespace scripting
} // namespace lc
