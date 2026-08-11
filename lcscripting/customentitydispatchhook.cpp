// Phase 6 PR-6.1 sub-piece 2b — CustomEntity dispatch-hook slot storage.
//
// Mirrors the process-global slot pattern from PR-6.1 sub-piece 1's
// `registerPluginHookSlot()` in `lcadpythonscript/bridge/py_lc.cpp`.

#include "customentitydispatchhook.h"

namespace lc {
namespace scripting {

namespace {

std::shared_ptr<CustomEntityDispatchHook>& hookSlot() {
    static std::shared_ptr<CustomEntityDispatchHook> h;
    return h;
}

} // namespace

void setCustomEntityDispatchHook(std::shared_ptr<CustomEntityDispatchHook> hook) {
    hookSlot() = std::move(hook);
}

std::shared_ptr<CustomEntityDispatchHook> customEntityDispatchHook() {
    return hookSlot();
}

} // namespace scripting
} // namespace lc
