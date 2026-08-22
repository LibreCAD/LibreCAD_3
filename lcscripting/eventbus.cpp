#include "eventbus.h"

namespace lc {
namespace scripting {

void EventBus::registerEvent(const std::string& name, ScriptCallback cb) {
    std::lock_guard<std::mutex> lk(_mu);
    _events[name].push_back(std::move(cb));
}

bool EventBus::deleteEvent(const std::string& name, const ScriptCallback& cb) {
    std::lock_guard<std::mutex> lk(_mu);
    auto it = _events.find(name);
    if (it == _events.end()) return false;

    auto& vec = it->second;
    for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
        if (*vit == cb) {
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

void EventBus::triggerEvent(const std::string& name, const ScriptValue& args) {
    // Copy-before-dispatch (luainterface.cpp:134): grab a snapshot of
    // the callback vector while the mutex is held, then release the
    // mutex so callbacks can safely re-enter to register/deregister
    // without deadlocking.
    std::vector<ScriptCallback> snapshot;
    {
        std::lock_guard<std::mutex> lk(_mu);
        auto it = _events.find(name);
        if (it == _events.end()) return;
        snapshot = it->second;   // copy
    }

    for (const auto& cb : snapshot) {
        // Adapter chooses shape: plain callable → f(event, args);
        // object-with-onEvent → obj.onEvent(event, args).  See the
        // PythonCallbackImpl / LuaCallbackImpl invokeEvent overrides.
        // Return values are discarded.
        (void) cb.invokeEvent(name, args);
    }
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
