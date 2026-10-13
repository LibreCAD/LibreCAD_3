#pragma once

// Bridge — lc.geo.* (mirrors lcadluascript/bridge/lc_geo.h). Populated in
// slice 1.4.

#include "../qt_keywords_push.h"
#include <pybind11/pybind11.h>
#include "../qt_keywords_pop.h"

namespace lc {
namespace python {

/// Register lc.geo.Coordinate/Base/Vector/Area/Arc/BezierBase/Bezier/
/// CubicBezier/Circle/Ellipse/Spline/Region on the given `lc.geo` sub-module.
///
/// Coordinate uses py::class_ (value type per plan decision 1); the remaining
/// classes are py::classh so shared_ptr crossings (through EntityDistance,
/// entity constructors, etc.) work.  This mirrors the "one holder per
/// hierarchy" rule with geo::Base at the root of the geo hierarchy.
void import_py_lc_geo_namespace(pybind11::module_& m_geo);

} // namespace python
} // namespace lc
