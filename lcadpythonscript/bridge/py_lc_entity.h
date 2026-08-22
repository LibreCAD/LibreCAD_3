#pragma once

// Bridge — lc.entity.* (mirrors lcadluascript/bridge/lc_entity.h). Slice 1.6.

#include <pybind11/pybind11.h>

namespace lc {
namespace python {

/// Register the entity hierarchy on lc.entity: ID, CADEntity, interfaces
/// (Snapable/Draggable/Splitable/UnmanagedDraggable/Tangentable), and every
/// concrete entity (Arc, Circle, Point, Ellipse, Line, LWVertex2D, LWPolyline,
/// Spline, TextBase, Text, MText, Image, Insert, CustomEntity, DimAligned,
/// DimAngular, DimDiametric, DimLinear, DimRadial, Dimension).  Also
/// registers `lc.EntityDispatch` — deferred from slice 1.3 because its
/// visit() overloads need the entity Python types available.
///
/// All py::classh — every entity crosses as shared_ptr<const T>.  The Day-1
/// spike (slice 1.1) validated the holder policy for the MI+virtual-base
/// hierarchy this file registers.
void import_py_lc_entity_namespace(pybind11::module_& m_lc,
                                   pybind11::module_& m_entity);

} // namespace python
} // namespace lc
