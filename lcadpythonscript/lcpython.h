#pragma once

// LCPython — Python scripting facade for LibreCAD 3.
//
// Mirrors lcadluascript/lclua.h in shape and vocabulary (parity is a HARD plan
// constraint: kernel bindings mirror the lc.* API 1:1).  The kernel-bindings
// PYBIND11_EMBEDDED_MODULE(lc, ...) definition lives in lcpython.cpp and pulls
// each namespace in via the per-file bridge exports declared in bridge/*.h.
//
// IMPORTANT — coding rule (see master plan decision 8):
//   NEVER `#include <Python.h>` here or in any bridge.  Only include pybind11
//   headers.  pybind11's `conduit/wrap_include_python_h.h` handles the MSVC
//   debug-links-release-python autolink workaround; a bare Python.h include
//   reintroduces the python3x_d.lib link problem.
//
// GIL policy (plan decision 7): after PythonInit, the interpreter releases the
// GIL permanently.  Every C++->Python entry point takes gil_scoped_acquire
// INSIDE this facade / the phase-4 adapters — never at call sites.  py::object
// destructors and copies must also acquire the GIL; the PyNamespace wrapper
// below RAII-handles that so callers never touch a bare py::object.
//
// Entity safety invariant (plan decision 1, holder policy):
//   pybind11 does not enforce constness on shared_ptr<const T>.  The kernel's
//   entities-are-immutable invariant is what makes py::classh safe here.
//   No mutating bindings are added for entity classes.

#include <pybind11/embed.h>
#include <pybind11/pytypes.h>

#include <cad/storage/document.h>

#include <memory>
#include <string>

namespace lc {
namespace python {

/**
 * @brief Process-global Python interpreter singleton.
 *
 * Heap-allocated and intentionally leaked — the interpreter is NEVER
 * finalized (plan decision 2). Finalize-during-static-destruction with
 * py::objects still held by other statics is UB-ordering; the phase-6
 * custom-entity manager is exactly such a static.  Trade-off: Python
 * atexit handlers never run.  Construct with signal handlers OFF so
 * Ctrl-C is not stolen from the Qt event loop.
 *
 * Interpreter restart is FORBIDDEN — embedded modules re-init safely but
 * third-party imports (numpy, ...) do not.
 */
class PythonInit {
public:
    /// Initialize the singleton (idempotent — safe to call repeatedly).
    static void initialize();
    /// Access the singleton; asserts initialize() has been called.
    static PythonInit& instance();

    PythonInit(const PythonInit&) = delete;
    PythonInit& operator=(const PythonInit&) = delete;

private:
    PythonInit();
    ~PythonInit() = default;
};

/**
 * @brief RAII wrapper around a per-context Python globals dict.
 *
 * Owns a `py::dict` and acquires the GIL for every mutation, copy, and
 * destruction — the plan's permanent-release GIL pattern (decision 7) says
 * py::object destructors and copies must acquire the GIL because the Qt main
 * thread runs GIL-free.  Callers hold PyNamespace; the facade takes it by
 * reference to read/write the underlying dict under the GIL.
 *
 * The class is intentionally NOT a py::object subclass: hiding the raw dict
 * behind a pImpl guarantees no caller can leak GIL-required work outside the
 * facade.  Move-only for the same reason (copying a dict handle needs the
 * GIL — done in the copy assignment below when needed).
 */
class PyNamespace {
public:
    PyNamespace();                         // GIL acquired internally.
    ~PyNamespace();                        // GIL acquired internally.
    PyNamespace(PyNamespace&&) noexcept;   // No GIL — moves the pImpl pointer.
    PyNamespace& operator=(PyNamespace&&) noexcept;
    PyNamespace(const PyNamespace&) = delete;
    PyNamespace& operator=(const PyNamespace&) = delete;

    // Internal accessor used by LCPython — under GIL only.
    pybind11::dict& dict();
    const pybind11::dict& dict() const;

    /// Inject a named value into the namespace under the GIL. Generic
    /// counterpart to LCPython::setDocument() for callers (tests, future
    /// GUI/CLI contexts) that need to seed a namespace with a C++ value
    /// before runString() — GIL handling stays encapsulated here per plan
    /// decision 7; callers must never touch dict() directly outside the GIL.
    template <typename T>
    void set(const char* key, T&& value) {
        pybind11::gil_scoped_acquire gil;
        dict()[key] = std::forward<T>(value);
    }

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

/**
 * @brief Python scripting facade — one instance per script context
 * (MainWindow, ScriptDock widget, headless CLI, plugin thread).
 *
 * Vocabulary mirrors lcadluascript/lclua.h:
 *   - `makeNamespace()`   — fresh globals with `import lc` pre-executed
 *                           (parity with LCLua::importLCKernel, but per-context)
 *   - `setDocument(ns, d)` — injects the name `document` into ns
 *                           (parity with LCLua::setDocument)
 *   - `runString(code, ns)`/`runFile` — catches py::error_already_set and
 *                           returns the traceback text as a std::string
 *                           (parity with LCLua::runString's error-text return)
 *
 * Lua's utility surface (microtime, openFile/FILE userdata, openFileDialog)
 * is deliberately NOT mirrored — Python has time/open; the CLI file-dialog
 * shim is N/A (see phase-1 sub-plan Facade design bullet).
 */
class LCPython {
public:
    LCPython();

    /// Fresh per-context globals with `__builtins__` set and `import lc`
    /// executed.  Never returns the interpreter-wide dict.
    PyNamespace makeNamespace();

    /// Inject exactly one name (`document`) into @p ns.
    void setDocument(PyNamespace& ns,
                     const lc::storage::Document_SPtr& document);

    /// Execute @p code in @p ns.  Returns the empty string on success, or the
    /// exception's traceback text on error (Lua parity — never throws to the
    /// caller from this level).
    std::string runString(const char* code, PyNamespace& ns);

    /// Execute the file at @p path.  Same return contract as runString().
    std::string runFile(const char* path, PyNamespace& ns);
};

} // namespace python
} // namespace lc
