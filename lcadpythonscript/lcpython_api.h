#pragma once

// Visibility marker for lcpythonscript functions that cross the shared-library
// boundary into lcUI (or the unit tests) with a pybind11 type in their
// signature.
//
// pybind11 marks its entire namespace hidden:
//
//   // detail/pybind11_namespace_macros.h
//   #define PYBIND11_NAMESPACE pybind11 __attribute__((visibility("hidden")))
//
// and GCC gives a symbol the MINIMUM of its own visibility and that of the
// types in its signature. A function taking `pybind11::object` — or a
// `std::function<...>` instantiated over one — is therefore left out of
// liblcpythonscript's dynamic symbol table entirely, and linking librecad fails
// with "undefined reference" even though the definition compiled fine.
//
// Clang does NOT demote a plain function this way: it honours the hidden
// attribute on pybind11's own namespace, but does not propagate hidden-ness
// from a signature type to the function. So a MISSING marker is invisible on
// macOS/Clang and on any Clang-built Linux tree, and shows up only as an
// undefined reference in the GCC CI. If you add a function here that takes or
// returns a pybind11 type and is called from lcUI, unittest or luacmdinterface,
// it needs this marker — a clean local Clang build does not prove otherwise.
//
// Marking these few boundary functions default-visibility exports them while
// leaving pybind11's hidden-visibility protection intact everywhere else, which
// matters because that protection is what keeps separately-built extension
// modules from clashing inside the embedded interpreter.
//
// lckernel/build_constants.h's EXPORT_API cannot be reused: it is
// __declspec(dllexport) on MSVC and empty everywhere else, so it does nothing
// on the GCC/Clang builds where this problem occurs.

#if defined(_WIN32)
#define LC_PYTHON_API __declspec(dllexport)
#elif defined(__GNUC__)
#define LC_PYTHON_API __attribute__((visibility("default")))
#else
#define LC_PYTHON_API
#endif
