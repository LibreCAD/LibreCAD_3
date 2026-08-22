#pragma once

// Phase 6 PR-6.1 sub-piece 2b — CustomEntity concrete-type-preserving
// dispatch hook interface.
//
// Rationale: the 6 script-defined behavior slots on `ScriptCustomEntity`
// (snap / nearestPoint / dragPoints / newDragPoint / dragPointClick /
// dragPointRelease) need CONCRETE-TYPE preservation for Lua callbacks
// (see the sub-piece 1 fixup and sub-piece 2a's dispatch pattern).
// The concrete-type-preserving path uses kaguya's static template
// deduction — which requires including kaguya's headers.  But
// `lcscripting` (which owns the ScriptCustomEntity + CustomEntityBuilder
// classes as of PR-6.1 sub-piece 2b's physical move) MUST NOT depend
// on kaguya or Lua per the phase-4 layering constraint (lcscripting sits
// below the language adapters).
//
// Solution: `lcscripting` defines this abstract interface with one
// method per dispatch site.  Each dispatch method on `ScriptCustomEntity`
// asks the installed hook to try the fast path FIRST; if the hook
// returns false, falls through to the neutral ScriptValue-based
// `callback.invoke(...)` path (which fires native/Python callbacks
// correctly per the sub-piece 2a fixup).
//
// `lcadluascript` provides `LuaCustomEntityDispatchHook`
// (`scriptadapter/customentitydispatch_lua.{h,cpp}`) — an implementation
// that uses `unwrapLuaCallback` + `luaRef.call<T>()` for the Lua fast
// path.  Installation happens at `LCLua` construction time via
// `setCustomEntityDispatchHook()`.
//
// Pattern reference: this mirrors `lc::python::setRegisterPluginHook`
// from PR-6.1 sub-piece 1 (Python `lc.register_plugin` implemented as a
// hook slot in `lcadpythonscript`, installed by lcUI at initLua time).

#include "scriptcallback.h"

#include <cad/geometry/geocoordinate.h>
#include <cad/interface/snapconstrain.h>
#include <cad/vo/entitycoordinate.h>

#include <map>
#include <memory>
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

/// Abstract interface implemented by language adapters (Lua today;
/// Python could add its own equivalent when concrete-type preservation
/// matters — though py::cast's RTTI-based polymorphic downcast handles
/// most cases for free).
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

/// Install the process-global hook.  Later installations OVERWRITE
/// earlier ones (matches the `setRegisterPluginHook` / `setEventRegisterHook`
/// policy from sub-piece 1 — one hook per process; adapters can
/// coordinate multi-language cases in their own logic).  Passing a
/// null shared_ptr clears the hook (falls back to neutral dispatch
/// only).
void setCustomEntityDispatchHook(std::shared_ptr<CustomEntityDispatchHook> hook);

/// Query the currently-installed hook.  Returns a null shared_ptr if
/// none is installed (headless mode / Lua adapter not linked).
std::shared_ptr<CustomEntityDispatchHook> customEntityDispatchHook();

} // namespace scripting
} // namespace lc
