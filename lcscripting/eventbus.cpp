#include "eventbus.h"

#include <exception>
#include <stdexcept>

namespace lc {
namespace scripting {

void EventBus::registerEvent(const std::string& name, ScriptCallback cb) {
    (void)registerEventTracked(name, std::move(cb));
}

EventBus::RegistrationId EventBus::registerEventTracked(
    const std::string& name, ScriptCallback cb) {
    std::lock_guard<std::mutex> lk(_mu);
    if (_nextRegistrationId == 0) {
        throw std::overflow_error("EventBus registration ID space exhausted");
    }
    const RegistrationId id = _nextRegistrationId++;
    _events[name].push_back(Registration{id, std::move(cb)});
    return id;
}

bool EventBus::deleteEvent(const std::string& name, const ScriptCallback& cb) {
    std::lock_guard<std::mutex> lk(_mu);
    auto it = _events.find(name);
    if (it == _events.end()) return false;

    auto& vec = it->second;
    for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
        if (vit->callback == cb) {
            vec.erase(vit);
            // Preserve verbatim: leave an empty vector in the map
            // rather than erasing the entry.  Lua's LuaInterface keeps
            // the key around too — some paths use `_events[name]` and
            // rely on the vector existing (empty is fine).
            return true;
        }
    }
    return false;
}

bool EventBus::deleteRegistration(const std::string& name, RegistrationId id) {
    std::lock_guard<std::mutex> lk(_mu);
    auto it = _events.find(name);
    if (it == _events.end()) return false;

    auto& entries = it->second;
    for (auto entry = entries.begin(); entry != entries.end(); ++entry) {
        if (entry->id == id) {
            entries.erase(entry);
            return true;
        }
    }
    return false;
}

void EventBus::triggerEvent(const std::string& name, const ScriptValue& args) {
    // Copy-before-dispatch (luainterface.cpp:134): grab a snapshot of
    // the callback vector while the mutex is held, then release the
    // mutex so callbacks can safely re-enter to register/deregister
    // without deadlocking.
    std::vector<Registration> snapshot;
    ScriptDiagnosticSink diagnostics;
    {
        std::lock_guard<std::mutex> lk(_mu);
        auto it = _events.find(name);
        if (it == _events.end()) return;
        snapshot = it->second;   // copy
        diagnostics = _diagnosticSink;
    }

    std::vector<RegistrationId> failedRegistrations;
    for (const auto& entry : snapshot) {
        bool callbackFailed = false;
        const ScriptDiagnosticSink callbackDiagnostics =
            [&callbackFailed, &diagnostics](const std::string& message) {
                callbackFailed = true;
                reportScriptDiagnostic(diagnostics, message);
            };
        // Adapter chooses shape: plain callable → f(event, args);
        // object-with-onEvent → obj.onEvent(event, args).  See the
        // PythonCallbackImpl / LuaCallbackImpl invokeEvent overrides.
        // Return values are discarded.
        try {
            (void) entry.callback.invokeEvent(name, args, callbackDiagnostics);
        } catch (const std::exception& error) {
            callbackDiagnostics(
                std::string("Script event '") + name + "' callback (" +
                    entry.callback.runtime() + ") failed: " + error.what());
        } catch (...) {
            callbackDiagnostics(
                std::string("Script event '") + name + "' callback (" +
                    entry.callback.runtime() + ") failed with an unknown exception");
        }
        if (callbackFailed) {
            failedRegistrations.push_back(entry.id);
        }
    }

    // Remove only after snapshot dispatch so callbacks can safely re-enter or
    // mutate the bus, and duplicate equal callbacks remain independent.
    for (const RegistrationId id : failedRegistrations) {
        deleteRegistration(name, id);
    }
}

void EventBus::setDiagnosticSink(ScriptDiagnosticSink sink) {
    std::lock_guard<std::mutex> lk(_mu);
    _diagnosticSink = std::move(sink);
}

ScriptDiagnosticSink EventBus::diagnosticSink() const {
    std::lock_guard<std::mutex> lk(_mu);
    return _diagnosticSink;
}

void EventBus::reportDiagnostic(const std::string& message) const {
    reportScriptDiagnostic(diagnosticSink(), message);
}

std::size_t EventBus::listenerCount(const std::string& name) const {
    std::lock_guard<std::mutex> lk(_mu);
    auto it = _events.find(name);
    return it == _events.end() ? 0u : it->second.size();
}

std::size_t EventBus::eventCount() const {
    std::lock_guard<std::mutex> lk(_mu);
    std::size_t n = 0;
    for (const auto& kv : _events) {
        if (!kv.second.empty()) ++n;
    }
    return n;
}

std::vector<ScriptCallback> EventBus::snapshot(const std::string& name) const {
    // Copy-before-return: matches the internal triggerEvent discipline
    // (grab the vector under the mutex, then hand it out so the caller
    // can iterate without holding the lock).
    const auto entries = snapshotEntries(name);
    std::vector<ScriptCallback> callbacks;
    callbacks.reserve(entries.size());
    for (const auto& entry : entries) {
        callbacks.push_back(entry.callback);
    }
    return callbacks;
}

std::vector<EventBus::Registration> EventBus::snapshotEntries(
    const std::string& name) const {
    std::lock_guard<std::mutex> lk(_mu);
    auto it = _events.find(name);
    if (it == _events.end()) return {};
    return it->second;
}

void EventBus::clear() {
    std::lock_guard<std::mutex> lk(_mu);
    _events.clear();
}

} // namespace scripting
} // namespace lc
