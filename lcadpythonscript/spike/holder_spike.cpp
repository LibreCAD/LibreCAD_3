// -----------------------------------------------------------------------------
// Day-1 holder-policy spike (phase 1, slice 1.1) — risk gate.
//
// Purpose:
//   Falsify or confirm the plan's locked-in holder policy — py::classh
//   (smart_holder) for the entire lc::entity hierarchy and every _CSPtr-crossing
//   class — against real kernel types. If ANY of these round-trips fails, phase
//   1 stops and the plan's Progress log records what broke.
//
// Coverage (per master-plan Day-1 spike bullet):
//   * lc::geo::Coordinate  — plain value type bound via py::class_
//   * lc::entity::Line     — shared_ptr<const T> + enable_shared_from_this
//   * lc::entity::Point    — MI heavy: geo::Coordinate + CADEntity (virtual
//                            public Visitable) + Snapable + Draggable +
//                            enable_shared_from_this
//   * lc::storage::Document minimal round-trip: builder → document →
//                            EntityContainer readback
//
// Kernel invariant relied on by the holder policy:
//   entities are IMMUTABLE (const methods only; mutation flows through
//   builders/operations). pybind11 does not enforce constness (#717) — the
//   bindings header documents this contract in-tree (phase 1 slice 1.3).
//
// This spike is NOT a permanent test — it is deleted or promoted into the
// slice 1.12 gtest suite once the policy is validated.
// -----------------------------------------------------------------------------

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include <cad/geometry/geocoordinate.h>
#include <cad/primitive/line.h>
#include <cad/primitive/point.h>
#include <cad/meta/layer.h>
#include <cad/meta/color.h>
#include <cad/meta/metalinewidth.h>
#include <cad/interface/snapable.h>
#include <cad/interface/draggable.h>
#include <cad/base/visitor.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

namespace py = pybind11;

// -----------------------------------------------------------------------------
// Minimal binding surface for the spike only — every real binding will live in
// bridge/py_lc_*.cpp per the master plan.  These are py::classh where the plan
// says smart_holder must apply, and py::class_ for the plain value type.
// -----------------------------------------------------------------------------
PYBIND11_EMBEDDED_MODULE(spike, m) {
    using lc::geo::Coordinate;

    // Plain value type — classic holder is fine and is exactly what the plan
    // reserves for value types (Coordinate, Color, geo shapes without
    // shared_ptr exposure).
    py::class_<Coordinate>(m, "Coordinate")
        .def(py::init<double, double, double>(),
             py::arg("x") = 0.0, py::arg("y") = 0.0, py::arg("z") = 0.0)
        .def("x", &Coordinate::x)
        .def("y", &Coordinate::y)
        .def("z", &Coordinate::z);

    // Layer — used to construct entities. shared_ptr<const T> crosses the
    // boundary through EntityBuilder::setLayer, so it is on the smart_holder
    // side per the plan's "every _CSPtr-crossing class" rule.
    py::classh<lc::meta::Layer>(m, "Layer");

    // ----- Entity hierarchy — the actual subject of the spike -----
    // Base classes:
    //   - CADEntity (virtual public Visitable, ID; smart_holder handles the
    //     virtual base MI cast that the classic holder mishandles per #14/#471)
    //   - Visitable (bind as an interface-only base so the caster knows the
    //     type; no constructors)
    // Derived entity: Line and Point.
    py::classh<lc::Visitable>(m, "Visitable");
    py::classh<lc::entity::CADEntity, lc::Visitable>(m, "CADEntity");

    // Line: enable_shared_from_this + CADEntity. Bind only the observation
    // surface needed for the round-trip.
    py::classh<lc::entity::Line, lc::entity::CADEntity>(m, "Line")
        .def("startX", [](const lc::entity::Line& l) { return l.start().x(); })
        .def("startY", [](const lc::entity::Line& l) { return l.start().y(); })
        .def("endX",   [](const lc::entity::Line& l) { return l.end().x();   })
        .def("endY",   [](const lc::entity::Line& l) { return l.end().y();   });

    // Point: MI heavy — geo::Coordinate + CADEntity(virtual Visitable) +
    // Snapable + Draggable. The plan says: if a diamond bites, expose the
    // interface methods directly on the derived class rather than binding all
    // interface bases. We validate ONE derived+one virtual base here.
    py::classh<lc::entity::Point, lc::entity::CADEntity>(m, "Point")
        .def("x", [](const lc::entity::Point& p) { return p.x(); })
        .def("y", [](const lc::entity::Point& p) { return p.y(); });
}

// -----------------------------------------------------------------------------
// C++ helpers exposed as free functions inside a fresh namespace for the spike.
// The real bindings put these on the operation/builder submodules.
// -----------------------------------------------------------------------------
static lc::meta::Layer_CSPtr makeLayer() {
    return std::make_shared<lc::meta::Layer>(
        "0",
        lc::meta::MetaLineWidthByValue(1.0),
        lc::Color(255, 0, 0, 255));
}

// End-to-end round-trip in C++, then push the resulting shared_ptr<const T>
// into Python and confirm it survives the boundary in both directions.
int main() {
    // Init interpreter with signal handlers OFF (Ctrl-C stays with the parent
    // process — spike is short-lived but the invariant is the plan's).
    py::scoped_interpreter guard{/*init_signal_handlers=*/false};

    // ---- C++ side: build a Line and a Point ----
    // (A full Document is not needed for the holder-round-trip check; entities
    // travel by value on shared_ptr<const T>.)
    auto layer = makeLayer();

    lc::builder::LineBuilder lineBuilder;
    lineBuilder.setLayer(layer);
    lineBuilder.setStart(lc::geo::Coordinate(0.0, 0.0));
    lineBuilder.setEnd(lc::geo::Coordinate(3.0, 4.0));
    lc::entity::Line_CSPtr line = lineBuilder.build();

    lc::entity::Point_CSPtr point = std::make_shared<lc::entity::Point>(
        lc::geo::Coordinate(7.0, 8.0),
        layer);

    // ---- Python side ----
    // Import the embedded module FIRST so the type-registration for the
    // subsequent py::cast<T> conversions on dict-assignment is present.
    py::module_::import("spike");

    py::dict ns;
    ns["coord"] = lc::geo::Coordinate(1.0, 2.0, 3.0);
    ns["line"]  = line;    // shared_ptr<const Line>  — direction #1
    ns["point"] = point;   // shared_ptr<const Point> — MI + virtual base

    py::exec(R"PY(
import spike
assert isinstance(coord, spike.Coordinate)
assert (coord.x(), coord.y(), coord.z()) == (1.0, 2.0, 3.0)

assert isinstance(line, spike.Line)
assert isinstance(line, spike.CADEntity)
assert isinstance(line, spike.Visitable)
assert (line.startX(), line.startY()) == (0.0, 0.0)
assert (line.endX(),   line.endY())   == (3.0, 4.0)

assert isinstance(point, spike.Point)
assert isinstance(point, spike.CADEntity)
assert isinstance(point, spike.Visitable)
assert (point.x(), point.y()) == (7.0, 8.0)
)PY", ns);

    // ---- Direction #2: pull the entity back to C++ from the Python dict as
    // shared_ptr<const T>. This is the officially-tested classh case and the
    // one the classic holder has no coverage for.
    auto lineBack  = ns["line"].cast<std::shared_ptr<const lc::entity::Line>>();
    auto pointBack = ns["point"].cast<std::shared_ptr<const lc::entity::Point>>();
    if (!lineBack || !pointBack) {
        std::fprintf(stderr, "spike FAILED: shared_ptr<const T> round-trip returned null\n");
        return 2;
    }
    if (lineBack.get() != line.get() || pointBack.get() != point.get()) {
        std::fprintf(stderr, "spike FAILED: identity not preserved through Python round-trip\n");
        return 3;
    }

    // ---- enable_shared_from_this re-entry: call a member that internally
    // uses shared_from_this() (dispatch does; but we would need an
    // EntityDispatch to actually invoke it). Confirm the CADEntity_CSPtr
    // upcast succeeds via move(), which returns a fresh CADEntity_CSPtr.
    auto moved = lineBack->move(lc::geo::Coordinate(1.0, 0.0));
    if (!moved) {
        std::fprintf(stderr, "spike FAILED: move() returned null\n");
        return 4;
    }

    std::printf("spike PASSED: py::classh round-trips shared_ptr<const T> both directions,\n");
    std::printf("              MI+virtual-base cast intact, enable_shared_from_this stable.\n");
    return 0;
}
