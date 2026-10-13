#pragma once

// Bridge — lc.meta.* (mirrors lcadluascript/bridge/lc_meta.h).  Slice 1.5.

#include "../qt_keywords_push.h"
#include <pybind11/pybind11.h>
#include "../qt_keywords_pop.h"

namespace lc {
namespace python {

/// Register the meta hierarchy on lc.meta: MetaType (root), DocumentMetaType,
/// EntityMetaType, DxfLinePattern + ByValue/ByBlock, MetaInfo, MetaColor +
/// ByValue/ByBlock, MetaLineWidth + ByValue/ByBlock, Block, Layer,
/// CustomEntityStorage.  All py::classh (whole hierarchy crosses _CSPtr
/// boundaries — layer/metaInfo/block flow into entity constructors).
void import_py_lc_meta_namespace(pybind11::module_& m_meta);

} // namespace python
} // namespace lc
