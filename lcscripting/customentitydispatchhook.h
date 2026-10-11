#pragma once

// Phase 6 PR-6.1 sub-piece 2b — CustomEntity concrete-type-preserving
// dispatch hook interface.
//
// Rationale: the 6 script-defined behavior slots on `ScriptCustomEntity`
// (snap / nearestPoint / dragPoints / newDragPoint / dragPointClick /
// dragPointRelease) need concrete argument/result preservation for language
// callbacks. Lua originally required this to preserve Insert's userdata type;
// Python also needs it for snap constraints, Builder transactions, and typed
// snap/drag results. Runtime headers stay in adapter implementations;
// `lcscripting` (which owns ScriptCustomEntity + CustomEntityBuilder) MUST NOT
// depend on kaguya, Lua, or pybind11.
//
// Solution: `lcscripting` defines this abstract interface with one
// method per dispatch site.  Each dispatch method on `ScriptCustomEntity`
// asks the installed hook to try the fast path FIRST; if the hook
// returns false, falls through to the neutral ScriptValue-based
// `callback.invoke(...)` path for callbacks whose values are representable
// there (notably native callbacks).
//
// `lcadluascript` provides `LuaCustomEntityDispatchHook`
// (`scriptadapter/customentitydispatch_lua.{h,cpp}`) — an implementation
// that uses `unwrapLuaCallback` + `luaRef.call<T>()` for the Lua fast
// path.  Installation happens at `LCLua` construction time via
// `setCustomEntityDispatchHook()`. `lcadpythonscript` provides the analogous
// typed hook and installs it during Python interpreter initialization.
//
// The registry stays below both adapters and contains no runtime-specific
// implementation; each adapter installs its own hook when its runtime starts.

#include "scriptcallback.h"

#include <cad/geometry/geocoordinate.h>
#include <cad/interface/snapconstrain.h>
#include <cad/vo/entitycoordinate.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

// Forward-declare kernel types the hook methods reference by pointer.
namespace lc {
namespace entity {
class Insert;
}
namespace operation {
class Builder;
using Builder_SPtr = std::shared_ptr<Builder>;
}
}

namespace lc {
namespace scripting {

/// Abstract interface implemented by language adapters. Hooks are selected
/// by callback runtime so registering Lua cannot replace Python (or vice
/// versa).
///
/// Every method returns `bool`:
///   * `true`  — the hook fired the callback; `out` populated for
///               return-valued methods; caller SKIPS the neutral
///               `callback.invoke()` fallback.
///   * `false` — the hook doesn't own this callback (e.g., a native
///               ScriptCallback where the fast path doesn't apply).
///               Caller falls through to `callback.invoke(...)`.
class CustomEntityDispatchHook {
public:
    virtual ~CustomEntityDispatchHook() = default;

    /// Stable callback runtime key, e.g. "lua" or "python".
    virtual const char* runtime() const = 0;

    virtual bool trySnapPoints(
        const ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        const lc::geo::Coordinate& coord,
        const lc::SimpleSnapConstrain& simpleSnapConstrain,
        double minDistanceToSnap,
        int maxNumberOfSnapPoints,
        std::vector<lc::EntityCoordinate>& out) = 0;

    virtual bool tryNearestPointOnPath(
        const ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        const lc::geo::Coordinate& coord,
        lc::geo::Coordinate& out) = 0;

    virtual bool tryDragPoints(
        const ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        std::map<unsigned int, lc::geo::Coordinate>& out) = 0;

    virtual bool trySetDragPoint(
        const ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        const lc::geo::Coordinate& position) = 0;

    virtual bool tryOnDragPointClick(
        const ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        lc::operation::Builder_SPtr builder,
        unsigned int point) = 0;

    virtual bool tryOnDragPointRelease(
        const ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        lc::operation::Builder_SPtr builder) = 0;
};

/// Install or replace the hook for its runtime. Repeated installation is
/// idempotent and leaves hooks for other runtimes untouched. Null hooks are
/// ignored; use clearCustomEntityDispatchHook for explicit removal.
void setCustomEntityDispatchHook(std::shared_ptr<CustomEntityDispatchHook> hook);

/// Remove one runtime's hook. Intended for adapter teardown/tests.
void clearCustomEntityDispatchHook(const std::string& runtime);

/// Find the hook that owns this callback. Returns null for nil callbacks or
/// when that runtime's adapter has not been installed.
std::shared_ptr<CustomEntityDispatchHook> customEntityDispatchHook(
    const ScriptCallback& callback);

/// Runtime-keyed lookup for adapter tests and explicit adapter management.
std::shared_ptr<CustomEntityDispatchHook> customEntityDispatchHook(
    const std::string& runtime);

} // namespace scripting
} // namespace lc
