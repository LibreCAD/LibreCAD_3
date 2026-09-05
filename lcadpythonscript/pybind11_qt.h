#pragma once

// Include pybind11 from a translation unit that also uses Qt.
//
// Qt defines `slots`, `signals` and `emit` as preprocessor macros, while
// CPython uses `slots` as an ordinary struct member name (PyType_Spec::slots).
// Whenever Qt is included first, Qt's empty `slots` macro deletes that member
// and the Python headers fail to parse:
//
//   python3.10/object.h:227: error: expected unqualified-id before ';' token
//   pybind11/detail/function_record_pyobject.h:90: too many initializers for 'PyType_Spec'
//
// Suppressing the macros only around the pybind11 includes (rather than
// building with QT_NO_KEYWORDS) keeps `signals:`/`slots:`/`emit` usable in the
// including file, which matters because Qt classes such as ScriptDock declare
// slots in the very headers that need pybind11.

#pragma push_macro("slots")
#pragma push_macro("signals")
#pragma push_macro("emit")
#undef slots
#undef signals
#undef emit

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#pragma pop_macro("emit")
#pragma pop_macro("signals")
#pragma pop_macro("slots")
