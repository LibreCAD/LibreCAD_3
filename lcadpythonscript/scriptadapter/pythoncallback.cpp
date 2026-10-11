#include "pythoncallback.h"

#include "../lcpython.h"   // for PythonInit — makes sure the interpreter is up

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include <mutex>
#include <limits>
#include <stdexcept>

namespace py = pybind11;
namespace lcs = lc::scripting;

namespace {

std::mutex opaqueMaterializerMutex;
lc::python::PythonOpaqueMaterializer opaqueMaterializer;

py::object materializeOpaqueLocked(const lcs::OpaquePtr& opaque) {
    if (opaque.ptr == nullptr) {
        return py::none();
    }

    lc::python::PythonOpaqueMaterializer materializer;
    {
        std::lock_guard<std::mutex> lock(opaqueMaterializerMutex);
        materializer = opaqueMaterializer;
    }
    if (!materializer) {
        throw std::runtime_error(
            "Python callback received an opaque UI value, but no UI "
            "materializer is installed");
    }
    return materializer(opaque);
}

// -----------------------------------------------------------------------------
// Conversion helpers.  Called under the GIL by pythonCallback pImpl.
// -----------------------------------------------------------------------------

py::object toPyLocked(const lcs::ScriptValue& v) {
    using K = lcs::ScriptValue::Kind;
    switch (v.kind()) {
    case K::Nil:            return py::none();
    case K::Bool:           return py::bool_(v.asBool());
    case K::Int:            return py::int_(v.asInt());
    case K::Double:         return py::float_(v.asDouble());
    case K::String:         return py::str(v.asString());
    case K::Coordinate:     return py::cast(v.asCoordinate());
    case K::Color:          return py::cast(v.asColor());
    case K::Entity:         return py::cast(v.asEntity());
    case K::CoordinateList: return py::cast(v.asCoordinateList());
    case K::EntityList:     return py::cast(v.asEntityList());
    case K::Opaque:
        return materializeOpaqueLocked(v.asOpaque());
    case K::MapKind: {
        py::dict d;
        if (v.asMap()) {
            for (const auto& kv : *v.asMap()) {
                d[py::str(kv.first)] = toPyLocked(kv.second);
            }
        }
        return d;
    }
    }
    return py::none();
}

lcs::ScriptValue fromPyLocked(py::handle o) {
    if (o.is_none()) return lcs::ScriptValue{};
    if (py::isinstance<py::bool_>(o))  return lcs::ScriptValue(o.cast<bool>());
    if (py::isinstance<py::int_>(o)) {
        const long long value = PyLong_AsLongLong(o.ptr());
        if (PyErr_Occurred()) {
            throw py::error_already_set();
        }
        if (value < std::numeric_limits<int>::min() ||
            value > std::numeric_limits<int>::max()) {
            throw py::value_error(
                "Python integer is outside the ScriptValue int range");
        }
        return lcs::ScriptValue(static_cast<int>(value));
    }
    if (py::isinstance<py::float_>(o)) return lcs::ScriptValue(o.cast<double>());
    if (py::isinstance<py::str>(o))    return lcs::ScriptValue(o.cast<std::string>());
    if (py::isinstance<py::dict>(o)) {
        auto m = lcs::makeMap();
        for (auto item : o.cast<py::dict>()) {
            if (!py::isinstance<py::str>(item.first)) {
                throw py::type_error(
                    "ScriptValue maps require string keys");
            }
            (*m)[item.first.cast<std::string>()] = fromPyLocked(item.second);
        }
        return lcs::ScriptValue(std::move(m));
    }
    if (PyList_Check(o.ptr()) || PyTuple_Check(o.ptr())) {
        const py::sequence sequence = py::reinterpret_borrow<py::sequence>(o);
        if (sequence.empty()) {
            throw py::type_error(
                "an empty Python sequence is ambiguous in ScriptValue; "
                "return a typed coordinate/entity list or None");
        }

        std::vector<lc::geo::Coordinate> coordinates;
        std::vector<lc::entity::CADEntity_CSPtr> entities;
        coordinates.reserve(sequence.size());
        entities.reserve(sequence.size());

        bool allCoordinates = true;
        bool allEntities = true;
        for (const py::handle& item : sequence) {
            // Point and other multiple-inheritance entity bindings can also
            // satisfy the Coordinate check, so entity membership wins.
            if (py::isinstance<lc::entity::CADEntity>(item)) {
                entities.push_back(item.cast<lc::entity::CADEntity_CSPtr>());
                allCoordinates = false;
            } else if (py::isinstance<lc::geo::Coordinate>(item)) {
                coordinates.push_back(item.cast<lc::geo::Coordinate>());
                allEntities = false;
            } else {
                allCoordinates = false;
                allEntities = false;
                break;
            }
        }
        if (allEntities) {
            return lcs::ScriptValue(std::move(entities));
        }
        if (allCoordinates) {
            return lcs::ScriptValue(std::move(coordinates));
        }
        throw py::type_error(
            "Python sequences must contain only lc.geo.Coordinate values "
            "or only lc.entity.CADEntity values to be represented in ScriptValue");
    }
    // Try the kernel-value casts.  If the object is a bound lc.* type,
    // pybind11 will succeed.  We use try/catch because pybind11 raises
    // cast_error on mismatch.
    try {
        return lcs::ScriptValue(o.cast<lc::geo::Coordinate>());
    } catch (const py::cast_error&) {}
    try {
        return lcs::ScriptValue(o.cast<lc::Color>());
    } catch (const py::cast_error&) {}
    try {
        return lcs::ScriptValue(o.cast<lc::entity::CADEntity_CSPtr>());
    } catch (const py::cast_error&) {}
    throw py::type_error(
        std::string("Python value of type '") + Py_TYPE(o.ptr())->tp_name +
        "' cannot be represented in ScriptValue");
}

// -----------------------------------------------------------------------------
// The pImpl.  Every op that touches py::object acquires the GIL.
// -----------------------------------------------------------------------------
class PythonCallbackImpl : public lcs::ScriptCallbackImpl {
public:
    explicit PythonCallbackImpl(py::object obj) {
        lc::python::PythonInit::initialize();
        // Store under GIL.
        py::gil_scoped_acquire gil;
        _obj = std::move(obj);
    }

    ~PythonCallbackImpl() override {
        // ~py::object needs the GIL (Py_DECREF).  Under permanent-release
        // the caller thread does NOT hold it; acquire here.
        py::gil_scoped_acquire gil;
        _obj = py::object();   // release the refcount under the GIL
    }

    // Copy is not defined on the base (we intentionally block it — the
    // ScriptCallback outer holds a shared_ptr; copying THAT is refcount
    // only.  If a caller ever needs a "new interior copy" they should
    // reconstruct via makePythonCallback(_obj).  This mirrors the plan's
    // GIL-in-copy note: no path in the current call sites needs a deep
    // copy).
    PythonCallbackImpl(const PythonCallbackImpl&)            = delete;
    PythonCallbackImpl& operator=(const PythonCallbackImpl&) = delete;

    const char* runtime() const override { return "python"; }

    py::object invokePython(const py::tuple& args) {
        py::gil_scoped_acquire gil;
        return _obj(*args);
    }

    lcs::ScriptValue invoke(const std::vector<lcs::ScriptValue>& args) override {
        lcs::ScriptValue result;
        std::string failure;
        try {
            py::gil_scoped_acquire gil;
            try {
                py::tuple pyArgs(args.size());
                for (std::size_t i = 0; i < args.size(); ++i) {
                    pyArgs[i] = toPyLocked(args[i]);
                }
                py::object pyResult = _obj(*pyArgs);
                result = fromPyLocked(pyResult);
            } catch (const py::error_already_set& error) {
                failure = error.what();
            } catch (const std::exception& error) {
                failure = error.what();
            } catch (...) {
                failure = "unknown Python callback failure";
            }
        } catch (const std::exception& error) {
            // GIL acquisition itself can fail during interpreter shutdown.
            failure = error.what();
        } catch (...) {
            failure = "unknown Python callback failure";
        }

        if (!failure.empty()) {
            lcs::reportScriptDiagnostic({}, "Python callback failed:\n" + failure);
        }
        return result;
    }

    lcs::ScriptValue invokeEvent(
        const std::string& event,
        const lcs::ScriptValue& args,
        const lcs::ScriptDiagnosticSink& diagnostics) override {
        lcs::ScriptValue result;
        std::string failure;
        {
            py::gil_scoped_acquire gil;
            try {
                // Python "onEvent" shape: object has a callable `onEvent`
                // attribute — call it as `obj.onEvent(event, args)`.
                //   * A plain callable → call as `obj(event, args)`.
                if (py::hasattr(_obj, "onEvent")) {
                    result = fromPyLocked(
                        _obj.attr("onEvent")(event, toPyLocked(args)));
                } else {
                    result = fromPyLocked(_obj(event, toPyLocked(args)));
                }
            } catch (const py::error_already_set& error) {
                failure = error.what();
            } catch (const std::exception& error) {
                failure = error.what();
            } catch (...) {
                failure = "unknown Python callback failure";
            }
        }

        if (!failure.empty()) {
            lcs::reportScriptDiagnostic(
                diagnostics,
                "Python event '" + event + "' callback failed:\n" + failure);
        }
        return result;
    }

    bool equals(const ScriptCallbackImpl& other) const override {
        auto* pother = dynamic_cast<const PythonCallbackImpl*>(&other);
        if (!pother) return false;
        py::gil_scoped_acquire gil;
        // Master plan design decision 4: is-or-equal, NOT identity alone.
        // Bound methods are fresh objects per access
        // (`self.on_point is self.on_point == False`), so `is`-only would
        // leak per-pixel mouseMove handlers.
        try {
            if (_obj.is(pother->_obj)) return true;
            // `a == b` in Python — pybind11 wraps this as PyObject_RichCompare.
            return _obj.equal(pother->_obj);
        } catch (py::error_already_set& e) {
            // Objects whose __eq__ raises — treat as unequal.  Log-only
            // fix candidate for later.
            e.restore();
            PyErr_Clear();
            return false;
        }
    }

private:
    py::object _obj;
};

} // namespace

namespace lc {
namespace python {

lc::scripting::ScriptCallback makePythonCallback(py::object obj) {
    return lc::scripting::ScriptCallback(
        std::make_shared<PythonCallbackImpl>(std::move(obj)));
}

py::object invokePythonCallback(const lc::scripting::ScriptCallback& callback,
                                const py::tuple& args) {
    auto* impl = dynamic_cast<PythonCallbackImpl*>(callback.impl().get());
    if (!impl) {
        throw std::invalid_argument(
            "invokePythonCallback requires a Python-backed ScriptCallback");
    }
    return impl->invokePython(args);
}

void setPythonOpaqueMaterializer(PythonOpaqueMaterializer materializer) {
    std::lock_guard<std::mutex> lock(opaqueMaterializerMutex);
    opaqueMaterializer = std::move(materializer);
}

py::object toPy(const lc::scripting::ScriptValue& v) {
    py::gil_scoped_acquire gil;
    return toPyLocked(v);
}

lc::scripting::ScriptValue fromPy(py::handle o) {
    py::gil_scoped_acquire gil;
    return fromPyLocked(o);
}

} // namespace python
} // namespace lc
