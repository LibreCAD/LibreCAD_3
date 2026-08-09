#include "pythoncallback.h"

#include "../lcpython.h"   // for PythonInit — makes sure the interpreter is up

#include <pybind11/embed.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace lcs = lc::scripting;

namespace {

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
        // We cannot materialize an unknown opaque type inside
        // lcadpythonscript (it doesn't know about lcUI types).  Emit
        // an int-carrying capsule with the tag so a lcUI-side extension
        // can decode.  For phase-4 PR-1 this is a placeholder — an
        // lcUI-side toPy override will replace it in phase-4 later PRs.
        return py::none();
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
    if (py::isinstance<py::int_>(o))   return lcs::ScriptValue(o.cast<int>());
    if (py::isinstance<py::float_>(o)) return lcs::ScriptValue(o.cast<double>());
    if (py::isinstance<py::str>(o))    return lcs::ScriptValue(o.cast<std::string>());
    if (py::isinstance<py::dict>(o)) {
        auto m = lcs::makeMap();
        for (auto item : o.cast<py::dict>()) {
            (*m)[py::str(item.first).cast<std::string>()] = fromPyLocked(item.second);
        }
        return lcs::ScriptValue(std::move(m));
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
    // Fall through — unmappable Python value.  Return Nil so the caller's
    // ScriptValue kind() shows Nil (typed accessors default).
    return lcs::ScriptValue{};
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

    lcs::ScriptValue invoke(const std::vector<lcs::ScriptValue>& args) override {
        py::gil_scoped_acquire gil;
        try {
            py::tuple pyArgs(args.size());
            for (std::size_t i = 0; i < args.size(); ++i) {
                pyArgs[i] = toPyLocked(args[i]);
            }
            py::object result = _obj(*pyArgs);
            return fromPyLocked(result);
        } catch (py::error_already_set& /*e*/) {
            // TODO(phase-4 later): route the traceback to CliCommand via
            // the EventBus's diagnostic sink once EventBus lands.  For
            // PR-1 the exception is caught to keep it from unwinding
            // through the Qt event loop (which is std::terminate).  See
            // the sub-plan's ScriptCallback GIL+lifetime bullet.
            //
            // py::error_already_set's destructor calls PyErr_Clear() for
            // us provided restore() was NOT called; keep the code path
            // minimal so we cannot accidentally re-raise.
            return lcs::ScriptValue{};
        }
    }

    lcs::ScriptValue invokeEvent(const std::string& event,
                                 const lcs::ScriptValue& args) override {
        py::gil_scoped_acquire gil;
        try {
            // Python "onEvent" shape: object has a callable `onEvent`
            // attribute — call it as `obj.onEvent(event, args)`.
            //   * A plain callable → call as `obj(event, args)`.
            if (py::hasattr(_obj, "onEvent")) {
                return fromPyLocked(_obj.attr("onEvent")(event, toPyLocked(args)));
            }
            return fromPyLocked(_obj(event, toPyLocked(args)));
        } catch (py::error_already_set& e) {
            e.restore();
            PyErr_Clear();
            return lcs::ScriptValue{};
        }
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
