#include "pythonobject.h"
#include "pythoncallback.h"          // for toPy/fromPy conversion helpers

#include "../lcpython.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace lcs = lc::scripting;

namespace {

// Forward-declared conversion helpers from pythoncallback.cpp.  We
// intentionally do NOT expose these publicly (they require the GIL held
// at the call site; safe adapter code calls them from inside a
// gil_scoped_acquire scope only).
py::object toPyLocked(const lcs::ScriptValue& v);
lcs::ScriptValue fromPyLocked(py::handle o);

// Copy from pythoncallback.cpp.  Duplicated intentionally: pybind11
// forbids externally-linked template helpers in different TUs for its
// type-registry lookups (they'd resolve in different translation-unit
// caches).  Keeping copies local avoids the "different type_info per
// TU" hazard the plan warns about in the pybind11 sub-DLL notes.
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
    case K::Opaque:         return py::none();
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
    try { return lcs::ScriptValue(o.cast<lc::geo::Coordinate>()); }
    catch (const py::cast_error&) {}
    try { return lcs::ScriptValue(o.cast<lc::Color>()); }
    catch (const py::cast_error&) {}
    try { return lcs::ScriptValue(o.cast<lc::entity::CADEntity_CSPtr>()); }
    catch (const py::cast_error&) {}
    return lcs::ScriptValue{};
}

class PythonObjectImpl : public lcs::ScriptObjectImpl {
public:
    explicit PythonObjectImpl(py::object obj) {
        lc::python::PythonInit::initialize();
        py::gil_scoped_acquire gil;
        _obj = std::move(obj);
    }

    ~PythonObjectImpl() override {
        py::gil_scoped_acquire gil;
        _obj = py::object();
    }

    PythonObjectImpl(const PythonObjectImpl&)            = delete;
    PythonObjectImpl& operator=(const PythonObjectImpl&) = delete;

    const char* runtime() const override { return "python"; }

    lcs::ScriptValue call(const std::vector<lcs::ScriptValue>& args) override {
        py::gil_scoped_acquire gil;
        try {
            return fromPyLocked(callRawLocked(args));
        } catch (py::error_already_set& /*e*/) {
            return lcs::ScriptValue{};
        }
    }

    std::shared_ptr<lcs::ScriptObjectImpl>
    instantiate(const std::vector<lcs::ScriptValue>& args) override {
        // Phase 4 PR-7: constructor-invocation returning a fresh
        // PythonObjectImpl.  Phase 5's Python operation classes will use
        // this to hand MainWindow::runOperation a real Python instance
        // whose methods stay callable — the Value round-trip that
        // `call` does would drop the Python identity.
        py::gil_scoped_acquire gil;
        try {
            py::object result = callRawLocked(args);
            if (result.is_none()) return nullptr;
            return std::make_shared<PythonObjectImpl>(std::move(result));
        } catch (py::error_already_set& /*e*/) {
            return nullptr;
        }
    }

    lcs::ScriptValue getAttr(const std::string& name) override {
        py::gil_scoped_acquire gil;
        try {
            if (!py::hasattr(_obj, name.c_str())) return lcs::ScriptValue{};
            return fromPyLocked(_obj.attr(name.c_str()));
        } catch (py::error_already_set& /*e*/) {
            return lcs::ScriptValue{};
        }
    }

    void setAttr(const std::string& name, const lcs::ScriptValue& value) override {
        py::gil_scoped_acquire gil;
        try {
            _obj.attr(name.c_str()) = toPyLocked(value);
        } catch (py::error_already_set& /*e*/) {
            // Some Python objects are frozen (dataclass frozen=True,
            // namedtuple, ...).  Silently swallow so the caller's
            // setAttr doesn't propagate a Python exception through Qt.
        }
    }

    bool hasAttr(const std::string& name) override {
        py::gil_scoped_acquire gil;
        try {
            return py::hasattr(_obj, name.c_str());
        } catch (py::error_already_set& /*e*/) {
            return false;
        }
    }

    lcs::ScriptValue callMethod(const std::string& name,
                                const std::vector<lcs::ScriptValue>& args) override {
        py::gil_scoped_acquire gil;
        try {
            if (!py::hasattr(_obj, name.c_str())) return lcs::ScriptValue{};
            py::object method = _obj.attr(name.c_str());
            py::tuple pyArgs(args.size());
            for (std::size_t i = 0; i < args.size(); ++i) {
                pyArgs[i] = toPyLocked(args[i]);
            }
            return fromPyLocked(method(*pyArgs));
        } catch (py::error_already_set& /*e*/) {
            return lcs::ScriptValue{};
        }
    }

    bool equals(const ScriptObjectImpl& other) const override {
        auto* pother = dynamic_cast<const PythonObjectImpl*>(&other);
        if (!pother) return false;
        py::gil_scoped_acquire gil;
        try {
            if (_obj.is(pother->_obj)) return true;
            return _obj.equal(pother->_obj);
        } catch (py::error_already_set& /*e*/) {
            return false;
        }
    }

private:
    // Requires the GIL held by the caller.  Packs args into a py::tuple
    // and invokes `_obj(*args)`.  Both `call` and `instantiate` route
    // through this so the packing / call site lives in one place.
    py::object callRawLocked(const std::vector<lcs::ScriptValue>& args) {
        py::tuple pyArgs(args.size());
        for (std::size_t i = 0; i < args.size(); ++i) {
            pyArgs[i] = toPyLocked(args[i]);
        }
        return _obj(*pyArgs);
    }

    py::object _obj;
};

} // namespace

namespace lc {
namespace python {

lc::scripting::ScriptObject makePythonObject(py::object obj) {
    return lc::scripting::ScriptObject(
        std::make_shared<PythonObjectImpl>(std::move(obj)));
}

} // namespace python
} // namespace lc
