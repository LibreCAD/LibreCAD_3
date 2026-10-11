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
#include "bridge/py_lc_operation.h"
#include "bridge/py_lc_maths.h"
#include "bridge/py_lc_event.h"
#include "scriptadapter/python_customentitydispatch.h"
#include "python_runtime_config.h"
#if USE_PERSISTENCE
#include "bridge/py_lc_persistence.h"
#endif

#include <pybind11/embed.h>

#include <cassert>
#include <algorithm>
#include <codecvt>
#include <fstream>
#include <locale>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

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
    auto m_operation = m.def_submodule("operation", "Operations (mirrors lc.operation)");
    auto m_maths = m.def_submodule("maths",     "Math helpers (mirrors lc.maths)");
    auto m_event = m.def_submodule("event",     "Events (mirrors lc.event)");
#if USE_PERSISTENCE
    auto m_persistence = m.def_submodule("persistence",
        "Persistence: DXF/DWG open+save (mirrors lc.persistence)");
#endif

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

    // Populate lc.operation — slice 1.9.
    lc::python::import_py_lc_operation_namespace(m_operation);

    // Populate lc.maths — slice 1.10.
    lc::python::import_py_lc_maths_namespace(m_maths);

    // Populate lc.event — slice 1.11.
    lc::python::import_py_lc_event_namespace(m_event);

#if USE_PERSISTENCE
    // Populate lc.persistence — phase 2 slice 2.6 (required, per plan:
    // feeds the phase-6 custom-entity DXF round-trip regression test).
    lc::python::import_py_lc_persistence_namespace(m_persistence);
#endif
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
std::mutex g_runtime_mutex;
bool g_initialization_started = false;

struct IsolatedRuntimeConfig {
    std::string home;
    std::vector<std::string> moduleSearchPaths;
};

bool g_has_isolated_runtime_config = false;
IsolatedRuntimeConfig g_isolated_runtime_config;

std::wstring toWide(const std::string& utf8) {
#ifdef _WIN32
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(utf8);
#else
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(utf8);
#endif
}

std::string joinPath(const std::string& root, const std::string& relative) {
    if (root.empty()) return relative;
    if (relative.empty()) return root;
    const char last = root.back();
    if (last == '/' || last == '\\') return root + relative;
    return root + "/" + relative;
}

bool isFile(const std::string& path) {
#ifdef _WIN32
    const std::wstring wide = toWide(path);
    return _waccess(wide.c_str(), 0) == 0;
#else
    return access(path.c_str(), F_OK) == 0;
#endif
}

bool hasInstalledPythonPackage(const std::string& applicationDirectory) {
    if (applicationDirectory.empty()) return false;

    const std::vector<std::string> packageRoots = {
        joinPath(applicationDirectory, "../lcUIPy"),
        joinPath(applicationDirectory, "../share/librecad/lcUIPy"),
        joinPath(applicationDirectory, "../Resources/lcUIPy"),
        joinPath(applicationDirectory, "../Resources/librecad/lcUIPy"),
    };
    return std::any_of(packageRoots.begin(), packageRoots.end(),
        [](const std::string& root) {
            return isFile(joinPath(root, "__init__.py"));
        });
}

bool selectStagedRuntime(const std::string& applicationDirectory,
                         IsolatedRuntimeConfig& runtime) {
    if (applicationDirectory.empty()) return false;
    const std::vector<std::string> roots = {
        joinPath(applicationDirectory, "python"),
        joinPath(applicationDirectory, "../share/librecad/python"),
        joinPath(applicationDirectory, "../Resources/python"),
        joinPath(applicationDirectory, "../Resources/librecad/python"),
        joinPath(applicationDirectory, "../Frameworks/python"),
    };

    for (const auto& root : roots) {
        const std::string stdlib = joinPath(root, LC_PYTHON_STAGED_STDLIB_REL);
        if (!isFile(joinPath(stdlib, "os.py"))) continue;

        runtime.home = root;
        runtime.moduleSearchPaths.clear();
        const std::vector<std::string> relativePaths = {
            LC_PYTHON_STAGED_STDLIB_REL,
            LC_PYTHON_STAGED_STDARCH_REL,
            LC_PYTHON_STAGED_DYNLIBS_REL,
        };
        for (const auto& relative : relativePaths) {
            const std::string path = joinPath(root, relative);
            if (!isFile(joinPath(path, "os.py")) &&
                !isFile(joinPath(path, "encodings/__init__.py")) &&
                relative != LC_PYTHON_STAGED_DYNLIBS_REL) {
                continue;
            }
            if (std::find(runtime.moduleSearchPaths.begin(),
                          runtime.moduleSearchPaths.end(), path) ==
                runtime.moduleSearchPaths.end()) {
                runtime.moduleSearchPaths.push_back(path);
            }
        }
        return true;
    }
    return false;
}

void checkPyStatus(PyStatus status, const char* operation) {
    if (PyStatus_Exception(status)) {
        const char* detail = status.err_msg ? status.err_msg : "unknown CPython error";
        throw std::runtime_error(std::string(operation) + ": " + detail);
    }
}

void setConfigString(PyConfig& config, wchar_t** field,
                     const std::string& value, const char* operation) {
    const std::wstring wide = toWide(value);
    checkPyStatus(PyConfig_SetString(&config, field, wide.c_str()), operation);
}

void appendModuleSearchPath(PyConfig& config, const std::string& path) {
    if (path.empty()) {
        return;
    }
    const std::wstring wide = toWide(path);
    checkPyStatus(PyWideStringList_Append(&config.module_search_paths, wide.c_str()),
                  "append Python module search path");
}

void initializeIsolatedInterpreter(const IsolatedRuntimeConfig& runtime) {
    PyPreConfig preconfig;
    PyPreConfig_InitIsolatedConfig(&preconfig);
    preconfig.utf8_mode = 1;
    checkPyStatus(Py_PreInitialize(&preconfig), "preinitialize Python in UTF-8 mode");

    PyConfig config;
    PyConfig_InitIsolatedConfig(&config);
    bool passedToPybind = false;
    try {
        // Keep the flags explicit: packaged/application execution must not
        // inherit host Python settings or add the current/executable dir.
        config.parse_argv = 0;
        config.install_signal_handlers = 0;
        config.use_environment = 0;
        config.site_import = 0;
        config.user_site_directory = 0;
        config.write_bytecode = 0;
        config.module_search_paths_set = 1;

        setConfigString(config, &config.home, runtime.home,
                        "set Python runtime home");
        for (const auto& path : runtime.moduleSearchPaths) {
            appendModuleSearchPath(config, path);
        }
        if (config.module_search_paths.length == 0) {
            throw std::runtime_error("isolated Python configuration has no module search paths");
        }
        if (Py_IsInitialized()) {
            throw std::runtime_error("Python was initialized before runtime configuration");
        }

        // pybind11's PyConfig overload clears config on success or failure.
        // add_program_dir_to_path=false prevents it from reintroducing the
        // executable directory after our explicit isolated configuration.
        passedToPybind = true;
        g_interpreter = new py::scoped_interpreter(&config, 0, nullptr, false);
    } catch (...) {
        if (!passedToPybind) {
            PyConfig_Clear(&config);
        }
        throw;
    }
}

} // namespace

void PythonInit::configureForApplication(
    const std::string& applicationDirectory) {
    IsolatedRuntimeConfig config;
    config.home = LC_PYTHON_RUNTIME_HOME;
    config.moduleSearchPaths = {
        LC_PYTHON_RUNTIME_STDLIB,
        LC_PYTHON_RUNTIME_STDARCH,
        LC_PYTHON_RUNTIME_DYNLIBS,
        LC_PYTHON_RUNTIME_SITELIB,
        LC_PYTHON_RUNTIME_SITEARCH,
    };
    const bool stagedRuntime =
        selectStagedRuntime(applicationDirectory, config);
    if (hasInstalledPythonPackage(applicationDirectory) && !stagedRuntime) {
        throw std::runtime_error(
            "installed lcUIPy package found, but its staged CPython runtime is missing beside the application");
    }
    if (stagedRuntime && config.moduleSearchPaths.empty()) {
        throw std::runtime_error(
            "staged Python runtime contains no module search paths");
    }

    std::lock_guard<std::mutex> lock(g_runtime_mutex);
    if (g_has_isolated_runtime_config) {
        if (g_isolated_runtime_config.home != config.home ||
            g_isolated_runtime_config.moduleSearchPaths != config.moduleSearchPaths) {
            throw std::logic_error("conflicting Python runtime configuration");
        }
        return;
    }
    if (g_initialization_started || Py_IsInitialized()) {
        throw std::logic_error(
            "Python runtime configuration must be selected before initialization");
    }
    g_isolated_runtime_config = std::move(config);
    g_has_isolated_runtime_config = true;
}

void PythonInit::initialize() {
    std::call_once(g_init_flag, [] {
        IsolatedRuntimeConfig runtime;
        bool isolated = false;
        {
            std::lock_guard<std::mutex> lock(g_runtime_mutex);
            g_initialization_started = true;
            isolated = g_has_isolated_runtime_config;
            runtime = g_isolated_runtime_config;
        }

        if (isolated) {
            initializeIsolatedInterpreter(runtime);
        } else {
            // Tests and developer clients which do not select an application
            // runtime retain normal system-Python discovery.
            g_interpreter = new py::scoped_interpreter(
                /*init_signal_handlers=*/false);
        }

        // Permanent-release GIL pattern (plan decision 7).  After this call
        // the main thread does NOT hold the GIL; every re-entry point (this
        // facade's run*, phase-4 adapters, etc.) must acquire.
        g_gil_release = new py::gil_scoped_release();

        // Keep Python and Lua custom-entity dispatch hooks independent.
        installPythonCustomEntityDispatchHook();
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
// pybind11 types have hidden visibility.  This private pImpl must match so
// GCC does not try to expose a class containing a hidden py::dict member.
struct LC_PYTHON_LOCAL PyNamespace::Impl {
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
