// py_lc_maths.cpp — Python port of lcadluascript/bridge/lc_maths.cpp.

#include "py_lc_maths.h"

#include <pybind11/stl.h>

#include <cad/math/equation.h>
#include <cad/math/intersect.h>

namespace py = pybind11;

namespace lc {
namespace python {

void import_py_lc_maths_namespace(py::module_& m_maths) {
    py::class_<lc::maths::Equation>(m_maths, "Equation")
        .def(py::init<>())
        .def(py::init<double, double, double, double, double, double>())
        .def(py::init<const std::vector<double>&>())
        .def("Coefficients", &lc::maths::Equation::Coefficients)
        .def("Matrix",       &lc::maths::Equation::Matrix)
        .def("flipXY",       &lc::maths::Equation::flipXY)
        .def("move",         &lc::maths::Equation::move)
        .def("rotate",
             py::overload_cast<double>(&lc::maths::Equation::rotate, py::const_))
        .def("rotate",
             py::overload_cast<const lc::geo::Coordinate&, double>(&lc::maths::Equation::rotate, py::const_))
        .def_static("rotationMatrix",  &lc::maths::Equation::rotationMatrix)
        .def_static("translateMatrix", &lc::maths::Equation::translateMatrix);

    // IntersectMany — 2 ctors (with and without Method + tolerance).
    py::class_<lc::maths::IntersectMany>(m_maths, "IntersectMany")
        .def(py::init<std::vector<lc::entity::CADEntity_CSPtr>>())
        .def(py::init<std::vector<lc::entity::CADEntity_CSPtr>,
                      lc::maths::Intersect::Method, double>())
        .def("result", &lc::maths::IntersectMany::result);

    py::class_<lc::maths::IntersectAgainstOthers>(m_maths, "IntersectAgainstOthers")
        .def(py::init<std::vector<lc::entity::CADEntity_CSPtr>,
                      std::vector<lc::entity::CADEntity_CSPtr>,
                      lc::maths::Intersect::Method, double>())
        .def("result", &lc::maths::IntersectAgainstOthers::result);
}

} // namespace python
} // namespace lc
