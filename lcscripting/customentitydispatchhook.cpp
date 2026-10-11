// Runtime-keyed CustomEntity dispatch-hook storage. The registry is process
// wide, but each scripting runtime owns an independent slot.

#include "customentitydispatchhook.h"

#include <map>
#include <mutex>
#include <string>

namespace lc {
namespace scripting {

namespace {

std::map<std::string, std::shared_ptr<CustomEntityDispatchHook>>& hookSlots() {
    static std::map<std::string, std::shared_ptr<CustomEntityDispatchHook>> hooks;
    return hooks;
}

std::mutex& hookSlotsMutex() {
    static std::mutex mutex;
    return mutex;
}

} // namespace

void setCustomEntityDispatchHook(std::shared_ptr<CustomEntityDispatchHook> hook) {
    if (!hook || !hook->runtime() || hook->runtime()[0] == '\0') {
        return;
    }
    const std::string runtime = hook->runtime();
    std::lock_guard<std::mutex> lock(hookSlotsMutex());
    hookSlots()[runtime] = std::move(hook);
}

void clearCustomEntityDispatchHook(const std::string& runtime) {
    std::lock_guard<std::mutex> lock(hookSlotsMutex());
    hookSlots().erase(runtime);
}

std::shared_ptr<CustomEntityDispatchHook> customEntityDispatchHook(
    const ScriptCallback& callback) {
    if (callback.isNil()) {
        return {};
    }
    return customEntityDispatchHook(callback.runtime());
}

std::shared_ptr<CustomEntityDispatchHook> customEntityDispatchHook(
    const std::string& runtime) {
    std::lock_guard<std::mutex> lock(hookSlotsMutex());
    const auto it = hookSlots().find(runtime);
    return it == hookSlots().end() ? nullptr : it->second;
}

} // namespace scripting
} // namespace lc
