#pragma once

// Bridge — lc.maths.* (mirrors lcadluascript/bridge/lc_maths.h).  Slice 1.10.

#include "../qt_keywords_push.h"
#include <pybind11/pybind11.h>
#include "../qt_keywords_pop.h"

namespace lc {
namespace python {

void import_py_lc_maths_namespace(pybind11::module_& m_maths);

} // namespace python
} // namespace lc
