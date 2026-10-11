#include "python_customentitydispatch.h"

#include "pythoncallback.h"
#include <cad/operations/builder.h>
#include <cad/primitive/insert.h>
#include "../qt_keywords_push.h"
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include "../qt_keywords_pop.h"

#include <iostream>
#include <cstring>

namespace py = pybind11;

namespace {

template <typename Fn>
bool invokePythonSlot(const lc::scripting::ScriptCallback& callback,
                      const char* slot,
                      Fn&& fn) {
    if (std::strcmp(callback.runtime(), "python") != 0) {
        return false;
    }

    // A Python callback is handled once even if it raises or returns an
    // invalid type. Returning true prevents the neutral fallback from
    // invoking the failing callback a second time.
    py::gil_scoped_acquire gil;
    try {
        fn();
    } catch (const py::error_already_set& error) {
        std::cerr << "Python ScriptCustomEntity " << slot << " callback failed: "
                  << error.what() << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Python ScriptCustomEntity " << slot
                  << " callback returned an invalid value: "
                  << error.what() << '\n';
    } catch (...) {
        std::cerr << "Python ScriptCustomEntity " << slot
                  << " callback failed with an unknown exception\n";
    }
    return true;
}

py::object invoke(const lc::scripting::ScriptCallback& callback,
                  py::tuple args) {
    return lc::python::invokePythonCallback(callback, args);
}

} // namespace

namespace lc {
namespace python {

bool PythonCustomEntityDispatchHook::trySnapPoints(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    const lc::geo::Coordinate& coord,
    const lc::SimpleSnapConstrain& simpleSnapConstrain,
    double minDistanceToSnap,
    int maxNumberOfSnapPoints,
    std::vector<lc::EntityCoordinate>& out) {
    out.clear();
    return invokePythonSlot(callback, "snapPoints", [&] {
        auto result = invoke(callback, py::make_tuple(
            py::cast(self), py::cast(coord), py::cast(simpleSnapConstrain),
            minDistanceToSnap, maxNumberOfSnapPoints));
        out = result.cast<std::vector<lc::EntityCoordinate>>();
    });
}

bool PythonCustomEntityDispatchHook::tryNearestPointOnPath(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    const lc::geo::Coordinate& coord,
    lc::geo::Coordinate& out) {
    out = coord;
    return invokePythonSlot(callback, "nearestPointOnPath", [&] {
        auto result = invoke(callback, py::make_tuple(py::cast(self), py::cast(coord)));
        out = result.cast<lc::geo::Coordinate>();
    });
}

bool PythonCustomEntityDispatchHook::tryDragPoints(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    std::map<unsigned int, lc::geo::Coordinate>& out) {
    out.clear();
    return invokePythonSlot(callback, "dragPoints", [&] {
        auto result = invoke(callback, py::make_tuple(py::cast(self)));
        out = result.cast<std::map<unsigned int, lc::geo::Coordinate>>();
    });
}

bool PythonCustomEntityDispatchHook::trySetDragPoint(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    const lc::geo::Coordinate& position) {
    return invokePythonSlot(callback, "setDragPoint", [&] {
        invoke(callback, py::make_tuple(py::cast(self), py::cast(position)));
    });
}

bool PythonCustomEntityDispatchHook::tryOnDragPointClick(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    lc::operation::Builder_SPtr builder,
    unsigned int point) {
    return invokePythonSlot(callback, "onDragPointClick", [&] {
        invoke(callback, py::make_tuple(
            py::cast(self), py::cast(builder), py::int_(point)));
    });
}

bool PythonCustomEntityDispatchHook::tryOnDragPointRelease(
    const lc::scripting::ScriptCallback& callback,
    const std::shared_ptr<const lc::entity::Insert>& self,
    lc::operation::Builder_SPtr builder) {
    return invokePythonSlot(callback, "onDragPointRelease", [&] {
        invoke(callback, py::make_tuple(py::cast(self), py::cast(builder)));
    });
}

void installPythonCustomEntityDispatchHook() {
    lc::scripting::setCustomEntityDispatchHook(
        std::make_shared<PythonCustomEntityDispatchHook>());
}

} // namespace python
} // namespace lc
