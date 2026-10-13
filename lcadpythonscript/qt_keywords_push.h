// Suppress Qt's keyword macros around a Python/pybind11 include.
//
// Qt defines `slots`, `signals` and `emit` as preprocessor macros, while CPython
// uses `slots` as an ordinary struct member name (PyType_Spec::slots).  A
// translation unit that has seen any Qt header before Python.h therefore fails
// to parse it:
//
//   python3.10/object.h:227: error: expected unqualified-id before ';' token
//   pybind11/detail/function_record_pyobject.h:90: too many initializers for 'PyType_Spec'
//
// Pair every Python-bearing include with qt_keywords_pop.h:
//
//   #include <qt_keywords_push.h>
//   #include <pybind11/pybind11.h>
//   #include <qt_keywords_pop.h>
//
// Guarding at each include site rather than relying on include ORDER is what
// makes this robust: pybind11's own headers are `#pragma once`, so whichever
// include reaches them first decides whether they parse correctly, and a guard
// that arrives second is silently inert.
//
// Restoring the macros afterwards (rather than building with QT_NO_KEYWORDS)
// keeps `signals:`/`slots:`/`emit` usable in the including file — ScriptDock
// declares slots in the very header that needs pybind11.
//
// Deliberately NOT `#pragma once`: this header must take effect at every use.

#pragma push_macro("slots")
#pragma push_macro("signals")
#pragma push_macro("emit")
#undef slots
#undef signals
#undef emit
