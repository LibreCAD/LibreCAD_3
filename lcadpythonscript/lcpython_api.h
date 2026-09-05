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
// and GCC/Clang give a symbol the MINIMUM of its own visibility and that of the
// types in its signature. A function taking `pybind11::object` — or a
// `std::function<...>` instantiated over one — is therefore left out of
// liblcpythonscript's dynamic symbol table entirely, and linking librecad fails
// with "undefined reference" even though the definition compiled fine.
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
