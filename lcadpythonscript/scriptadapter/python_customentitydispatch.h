#pragma once

#include "../lcpython_api.h"

#include <lcscripting/customentitydispatchhook.h>

namespace lc {
namespace python {

/// Python implementation of the typed ScriptCustomEntity callback path.
/// It retains concrete Insert identity and values that ScriptValue cannot
/// currently represent, while keeping pybind11 out of lcscripting.
class PythonCustomEntityDispatchHook
    : public lc::scripting::CustomEntityDispatchHook {
public:
    const char* runtime() const override { return "python"; }

    bool trySnapPoints(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        const lc::geo::Coordinate& coord,
        const lc::SimpleSnapConstrain& simpleSnapConstrain,
        double minDistanceToSnap,
        int maxNumberOfSnapPoints,
        std::vector<lc::EntityCoordinate>& out) override;

    bool tryNearestPointOnPath(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        const lc::geo::Coordinate& coord,
        lc::geo::Coordinate& out) override;

    bool tryDragPoints(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        std::map<unsigned int, lc::geo::Coordinate>& out) override;

    bool trySetDragPoint(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        const lc::geo::Coordinate& position) override;

    bool tryOnDragPointClick(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        lc::operation::Builder_SPtr builder,
        unsigned int point) override;

    bool tryOnDragPointRelease(
        const lc::scripting::ScriptCallback& callback,
        const std::shared_ptr<const lc::entity::Insert>& self,
        lc::operation::Builder_SPtr builder) override;
};

/// Install or replace the Python runtime hook. Safe to call repeatedly.
LC_PYTHON_API void installPythonCustomEntityDispatchHook();

} // namespace python
} // namespace lc
