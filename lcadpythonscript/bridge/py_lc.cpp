// py_lc.cpp — Python port of lcadluascript/bridge/lc.cpp.
//
// Populates the top-level `lc` module with the Lua-side lc.Visitable /
// lc.Color / lc.EntityCoordinate / lc.SimpleSnapConstrain / lc.EntityDistance
// bindings, in strict 1:1 name/method parity with the Lua binding.
//
// EntityDispatch is bound later — see the file header comment of py_lc.h.

#include "py_lc.h"

#include <cad/meta/color.h>
#include <cad/base/visitor.h>
#include <cad/vo/entitycoordinate.h>
#include <cad/interface/snapconstrain.h>
#include <cad/vo/entitydistance.h>
#include <cad/base/cadentity.h>

namespace py = pybind11;

namespace lc {
namespace python {

void import_py_lc_namespace(py::module_& m) {
    // ------------------------------------------------------------------------
    // lc.Visitable — abstract interface (no constructors). Bound with
    // py::classh because entity classes derive from it via `virtual public
    // Visitable` and must round-trip via shared_ptr<const T> — plan decision 1
    // holder policy: "one holder per hierarchy, no mixing".
    // ------------------------------------------------------------------------
    py::classh<lc::Visitable>(m, "Visitable");
    // NOTE: The Lua binding adds .addFunction("accept", &Visitable::accept),
    // but accept() takes a GeoEntityVisitor& — an interface with no Python
    // bindings today (Lua users pass in kaguya callback tables that the Lua
    // side wires through a trampoline; kaguya's `LuaRef` polymorphism handles
    // it).  Wiring an equivalent trampoline is deferred to phase 4 (neutral
    // callback layer) — same rationale as EntityDispatch above.

    // ------------------------------------------------------------------------
    // lc.Color — plain value type (py::class_, per plan decision 1).
    // ------------------------------------------------------------------------
    py::class_<lc::Color>(m, "Color")
        .def(py::init<>())
        .def(py::init<int, int, int, int>(),
             py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 0xff)
        .def(py::init<double, double, double, double>(),
             py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 1.0)
        .def(py::init<const lc::Color&>())
        .def("alpha",  &lc::Color::alpha)
        .def("alphaI", &lc::Color::alphaI)
        .def("blue",   &lc::Color::blue)
        .def("blueI",  &lc::Color::blueI)
        .def("green",  &lc::Color::green)
        .def("greenI", &lc::Color::greenI)
        .def("red",    &lc::Color::red)
        .def("redI",   &lc::Color::redI);

    // ------------------------------------------------------------------------
    // lc.EntityCoordinate — value object (Coordinate + point id).
    // ------------------------------------------------------------------------
    py::class_<lc::EntityCoordinate>(m, "EntityCoordinate")
        .def(py::init<const lc::geo::Coordinate&, int>(),
             py::arg("point"), py::arg("pointId"))
        .def(py::init<const lc::EntityCoordinate&>())
        .def("coordinate", &lc::EntityCoordinate::coordinate)
        .def("pointId",    &lc::EntityCoordinate::pointId);

    // ------------------------------------------------------------------------
    // lc.SimpleSnapConstrain — value bit-mask + angle + divisions.
    // Class-level constants exposed as .NONE/.ON_ENTITY/... (parity with
    // kaguya's implicit member exposure).
    // ------------------------------------------------------------------------
    py::class_<lc::SimpleSnapConstrain> ssc(m, "SimpleSnapConstrain");
    ssc.def(py::init<>())
       .def(py::init<uint16_t, int, double>(),
            py::arg("constrain"), py::arg("divisions"), py::arg("angle"))
       .def("angle",             &lc::SimpleSnapConstrain::angle)
       .def("constrain",         &lc::SimpleSnapConstrain::constrain)
       .def("disableConstrain",  &lc::SimpleSnapConstrain::disableConstrain)
       .def("divisions",         &lc::SimpleSnapConstrain::divisions)
       .def("enableConstrain",   &lc::SimpleSnapConstrain::enableConstrain)
       .def("hasConstrain",      &lc::SimpleSnapConstrain::hasConstrain)
       .def("setAngle",          &lc::SimpleSnapConstrain::setAngle)
       .def("setDivisions",      &lc::SimpleSnapConstrain::setDivisions);
    ssc.attr("NONE")            = py::int_(lc::SimpleSnapConstrain::NONE);
    ssc.attr("ON_ENTITY")       = py::int_(lc::SimpleSnapConstrain::ON_ENTITY);
    ssc.attr("ON_ENTITYPATH")   = py::int_(lc::SimpleSnapConstrain::ON_ENTITYPATH);
    ssc.attr("ENTITY_CENTER")   = py::int_(lc::SimpleSnapConstrain::ENTITY_CENTER);
    ssc.attr("LOGICAL")         = py::int_(lc::SimpleSnapConstrain::LOGICAL);
    ssc.attr("DIVIDED")         = py::int_(lc::SimpleSnapConstrain::DIVIDED);

    // ------------------------------------------------------------------------
    // lc.EntityDistance — pairs a CADEntity_CSPtr with a coordinate.
    // Takes a shared_ptr<const CADEntity> — the smart_holder round-trip we
    // validated in the Day-1 spike (slice 1.1).
    // ------------------------------------------------------------------------
    py::class_<lc::EntityDistance>(m, "EntityDistance")
        .def(py::init<lc::entity::CADEntity_CSPtr, const lc::geo::Coordinate&>(),
             py::arg("cadEntity"), py::arg("coordinate"))
        .def("coordinate", &lc::EntityDistance::coordinate)
        .def("entity",     &lc::EntityDistance::entity);
}

} // namespace python
} // namespace lc
