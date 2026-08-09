// LCPython facade implementation.  Per phase-1 sub-plan.
//
// The embedded module `lc` and its submodules geo/entity/builder/storage/
// operation/meta/maths/event are pulled in from bridge/py_lc_*.cpp files added
// in slices 1.3 through 1.11.  Until those exist, the module is empty except
// for its version string; the CI-visible `import lc` round-trip is checked in
// slice 1.12's gtest suite.

#include "lcpython.h"

#include "bridge/py_lc.h"
#include "bridge/py_lc_geo.h"
#include "bridge/py_lc_meta.h"
#include "bridge/py_lc_entity.h"
#include "bridge/py_lc_builder.h"
#include "bridge/py_lc_storage.h"

#include <pybind11/embed.h>

#include <cassert>
#include <memory>
#include <mutex>
#include <string>

namespace py = pybind11;

// -----------------------------------------------------------------------------
// PYBIND11_EMBEDDED_MODULE(lc, m) — the LibreCAD kernel binding module.
//
// PYBIND11_EMBEDDED_MODULE registration runs as a load-time static
// initializer feeding PyImport_AppendInittab — it MUST run before interpreter
// init.  Because lcpythonscript is a link-time dependency of every consumer
// (LCLua adapter aside), it is safe.  Adding modules after Py_Initialize is a
// hard error.
//
// Exactly one loaded image per process may contain this module (invariant
// verified against pybind11/embed.h).  Rule enforced by convention today:
// - `librecad` does NOT link `lcui` (it recompiles the sources), and the test
//   binary links `lcui` without recompiling.  Do NOT link lcui into librecad.
// - Do NOT convert lcpythonscript to a STATIC library — static-lib embedded
//   modules are silently dropped by the linker (pybind11 discussion #4619).
//
// Kernel types are bound HERE only.  Phase 3's `lcgui` module must never
// re-bind any of them (duplicate registration throws at import).
//
// Sub-modules will be populated slice 1.3 onward; for now the module exists so
// that `import lc` works from the embedded interpreter and other libraries
// can start linking against lcpythonscript.
// -----------------------------------------------------------------------------
PYBIND11_EMBEDDED_MODULE(lc, m) {
    m.doc() = "LibreCAD 3 kernel bindings — phase 1 in progress; "
              "submodules populated by bridge/py_lc_*.cpp per plan slices "
              "1.3..1.11.";

    // Sub-modules — kept in strict parity with the Lua-side namespaces.
    // Bridge exports (`import_py_lc_<ns>(py::module_&)`) added slice-by-slice.
    auto m_geo = m.def_submodule("geo",       "Geometry primitives (mirrors lc.geo)");
    auto m_meta = m.def_submodule("meta",      "Meta types (mirrors lc.meta)");
    auto m_entity = m.def_submodule("entity",    "Entities (mirrors lc.entity)");
    auto m_builder = m.def_submodule("builder",   "Builders (mirrors lc.builder)");
    auto m_storage = m.def_submodule("storage",   "Storage / Document (mirrors lc.storage)");
    m.def_submodule("operation", "Operations (mirrors lc.operation)");
    m.def_submodule("maths",     "Math helpers (mirrors lc.maths)");
    m.def_submodule("event",     "Events (mirrors lc.event)");

    // Populate top-level names (Visitable/Color/EntityCoordinate/
    // SimpleSnapConstrain/EntityDistance) — slice 1.3.
    lc::python::import_py_lc_namespace(m);

    // Populate lc.geo — slice 1.4.
    lc::python::import_py_lc_geo_namespace(m_geo);

    // Populate lc.meta — slice 1.5.
    lc::python::import_py_lc_meta_namespace(m_meta);

    // Populate lc.entity + lc.EntityDispatch — slice 1.6.
    lc::python::import_py_lc_entity_namespace(m, m_entity);

    // Populate lc.builder — slice 1.7.
    lc::python::import_py_lc_builder_namespace(m_builder);

    // Populate lc.storage — slice 1.8.
    lc::python::import_py_lc_storage_namespace(m_storage);
}

namespace lc {
namespace python {

// -----------------------------------------------------------------------------
// PythonInit — leaked-singleton interpreter.
// -----------------------------------------------------------------------------
namespace {

// Deliberately leaked (see plan decision 2 — no finalize).
py::scoped_interpreter* g_interpreter = nullptr;
// The permanent-release lifetime: after init, the Qt main thread runs
// GIL-free.  Every C++->Python entry uses gil_scoped_acquire.
py::gil_scoped_release* g_gil_release = nullptr;

std::once_flag g_init_flag;

} // namespace

void PythonInit::initialize() {
    std::call_once(g_init_flag, [] {
        // Signal handlers OFF — the default installs CPython's SIGINT handler,
        // which makes Ctrl-C a no-op for a Qt-event-loop process.
        // Explicit heap-allocation ties the lifetime to the process, matching
        // the "intentionally leaked, never finalized" decision.
        g_interpreter = new py::scoped_interpreter(
            /*init_signal_handlers=*/false);

        // Permanent-release GIL pattern (plan decision 7).  After this call
        // the main thread does NOT hold the GIL; every re-entry point (this
        // facade's run*, phase-4 adapters, etc.) must acquire.
        g_gil_release = new py::gil_scoped_release();
    });
}

PythonInit& PythonInit::instance() {
    initialize();
    static PythonInit s_singleton;
    return s_singleton;
}

PythonInit::PythonInit() = default;

// -----------------------------------------------------------------------------
// PyNamespace — RAII wrapper acquiring the GIL for construction/destruction/
// mutation of the underlying py::dict.  The GIL is NOT held on the Qt main
// thread (permanent-release pattern), so any py::object dtor must acquire it
// explicitly or crash on Py_DECREF.
// -----------------------------------------------------------------------------
struct PyNamespace::Impl {
    py::dict dict;
};

PyNamespace::PyNamespace() {
    // Ensure the interpreter is up before creating the dict.
    PythonInit::initialize();
    py::gil_scoped_acquire gil;
    _impl = std::make_unique<Impl>();
}

PyNamespace::~PyNamespace() {
    if (_impl) {
        py::gil_scoped_acquire gil;
        _impl.reset();
    }
}

PyNamespace::PyNamespace(PyNamespace&& other) noexcept
    : _impl(std::move(other._impl)) {}

PyNamespace& PyNamespace::operator=(PyNamespace&& other) noexcept {
    if (this != &other) {
        if (_impl) {
            py::gil_scoped_acquire gil;
            _impl.reset();
        }
        _impl = std::move(other._impl);
    }
    return *this;
}

py::dict& PyNamespace::dict() {
    assert(_impl && "PyNamespace has been moved-from");
    return _impl->dict;
}

const py::dict& PyNamespace::dict() const {
    assert(_impl && "PyNamespace has been moved-from");
    return _impl->dict;
}

// -----------------------------------------------------------------------------
// LCPython facade — parity with lcadluascript/lclua.cpp shape.
// -----------------------------------------------------------------------------
LCPython::LCPython() {
    PythonInit::initialize();
}

PyNamespace LCPython::makeNamespace() {
    PyNamespace ns;
    py::gil_scoped_acquire gil;
    // __builtins__ must be present for any exec/eval to work.
    ns.dict()["__builtins__"] = py::module_::import("builtins");
    // `import lc` — the embedded module.  Every namespace has it pre-loaded
    // so scripts can start with `lc.geo.Coordinate(...)`.
    py::exec("import lc", ns.dict());
    return ns;
}

void LCPython::setDocument(PyNamespace& ns,
                           const lc::storage::Document_SPtr& document) {
    py::gil_scoped_acquire gil;
    // Parity with LCLua::setDocument (lclua.cpp:70-73): inject ONE name only.
    // Never bind mainWindow/document at interpreter or module scope — that
    // is per-context state and would leak across MainWindows.
    ns.dict()["document"] = document;
}

std::string LCPython::runString(const char* code, PyNamespace& ns) {
    py::gil_scoped_acquire gil;
    try {
        py::exec(code, ns.dict());
    } catch (py::error_already_set& e) {
        // Parity with LCLua::runString: return the error text; never throw.
        // pybind11's error_already_set::what() renders the exception + type;
        // callers stringify it into the console/log.
        return std::string(e.what());
    }
    return {};
}

std::string LCPython::runFile(const char* path, PyNamespace& ns) {
    py::gil_scoped_acquire gil;
    try {
        py::eval_file(path, ns.dict());
    } catch (py::error_already_set& e) {
        return std::string(e.what());
    }
    return {};
}

} // namespace python
} // namespace lc
