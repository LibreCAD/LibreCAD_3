#pragma once

// Bridge — lc.builder.* (mirrors lcadluascript/bridge/lc_builder.h).  Slice 1.7.

#include "../qt_keywords_push.h"
#include <pybind11/pybind11.h>
#include "../qt_keywords_pop.h"

namespace lc {
namespace python {

/// Register all *Builder classes on lc.builder: LinePatternBuilder,
/// LayerBuilder, CADEntityBuilder (base for all entity builders), and every
/// per-entity builder (Arc, Circle, Dimension + subtypes, Point, Ellipse,
/// Line, Spline, LWPolyline, Insert, TextBase, Text, MText).
///
/// Builders are used from Python by construction, mutation via set*(), then
/// build() returns a shared_ptr<const T>.  py::class_ is fine for builders
/// (they aren't shared_ptr-passed across the boundary) — but every
/// CADEntityBuilder subclass takes/returns _CSPtr in setters/getters, so the
/// registered types must be classh so the ARGUMENT types resolve.
void import_py_lc_builder_namespace(pybind11::module_& m_builder);

} // namespace python
} // namespace lc
