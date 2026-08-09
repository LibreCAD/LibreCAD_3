#pragma once

// Bridge — lc.event.* (mirrors lcadluascript/bridge/lc_event.h).  Slice 1.11.

#include <pybind11/pybind11.h>

namespace lc {
namespace python {

void import_py_lc_event_namespace(pybind11::module_& m_event);

} // namespace python
} // namespace lc
