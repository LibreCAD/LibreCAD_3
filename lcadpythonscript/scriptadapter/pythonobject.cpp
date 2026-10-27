#include "pythonobject.h"
#include "pythoncallback.h"          // for toPy/fromPy conversion helpers

#include "../lcpython.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <exception>

namespace py = pybind11;
namespace lcs = lc::scripting;

namespace {

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
        lcs::ScriptValue result;
        std::string diagnostic;
        {
            py::gil_scoped_acquire gil;
            try {
                result = lc::python::fromPy(callRawLocked(args));
            } catch (py::error_already_set& exception) {
                diagnostic =
                    "Python callable failed or returned an unsupported value:\n" +
                    std::string(exception.what());
            } catch (const std::exception& exception) {
                diagnostic =
                    "Python callable returned an unsupported value: " +
                    std::string(exception.what());
            } catch (...) {
                diagnostic = "Python callable returned an unsupported value";
            }
        }
        if (!diagnostic.empty()) {
            lcs::reportScriptDiagnostic({}, diagnostic);
        }
        return result;
    }

    std::shared_ptr<lcs::ScriptObjectImpl>
    instantiate(const std::vector<lcs::ScriptValue>& args) override {
        // Phase 4 PR-7: constructor-invocation returning a fresh
        // PythonObjectImpl.  Phase 5's Python operation classes will use
        // this to hand MainWindow::runOperation a real Python instance
        // whose methods stay callable — the Value round-trip that
        // `call` does would drop the Python identity.
        std::shared_ptr<lcs::ScriptObjectImpl> result;
        std::string error;
        (void)instantiateChecked(args, result, error);
        return result;
    }

    bool instantiateChecked(
        const std::vector<lcs::ScriptValue>& args,
        std::shared_ptr<lcs::ScriptObjectImpl>& result,
        std::string& error) override {
        py::gil_scoped_acquire gil;
        result.reset();
        error.clear();
        try {
            py::object instance = callRawLocked(args);
            if (instance.is_none()) {
                error = "Python operation constructor returned None";
                return false;
            }
            result = std::make_shared<PythonObjectImpl>(std::move(instance));
            return true;
        } catch (py::error_already_set& exception) {
            error = exception.what();
        } catch (const std::exception& exception) {
            error = exception.what();
        } catch (...) {
            error = "Python operation constructor failed with an unknown exception";
        }
        return false;
    }

    lcs::ScriptValue getAttr(const std::string& name) override {
        lcs::ScriptValue result;
        std::string diagnostic;
        {
            py::gil_scoped_acquire gil;
            try {
                if (py::hasattr(_obj, name.c_str())) {
                    result = lc::python::fromPy(_obj.attr(name.c_str()));
                }
            } catch (py::error_already_set& exception) {
                diagnostic = "Python attribute '" + name +
                             "' could not be read or converted:\n" +
                             std::string(exception.what());
            } catch (const std::exception& exception) {
                diagnostic = "Python attribute '" + name +
                             "' has an unsupported value: " + exception.what();
            } catch (...) {
                diagnostic = "Python attribute '" + name +
                             "' has an unsupported value";
            }
        }
        if (!diagnostic.empty()) {
            lcs::reportScriptDiagnostic({}, diagnostic);
        }
        return result;
    }

    void setAttr(const std::string& name, const lcs::ScriptValue& value) override {
        std::string error;
        if (!setAttrChecked(name, value, error)) {
            lcs::reportScriptDiagnostic(
                {}, "Python attribute '" + name + "' could not be set:\n" + error);
        }
    }

    bool setAttrChecked(const std::string& name,
                        const lcs::ScriptValue& value,
                        std::string& error) override {
        py::gil_scoped_acquire gil;
        error.clear();
        try {
            _obj.attr(name.c_str()) = lc::python::toPy(value);
            return true;
        } catch (py::error_already_set& exception) {
            error = exception.what();
        } catch (const std::exception& exception) {
            error = exception.what();
        } catch (...) {
            error = "Python attribute assignment failed with an unknown exception";
        }
        return false;
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
        lcs::ScriptValue result;
        std::string error;
        (void)callMethodChecked(name, args, result, error);
        return result;
    }

    bool callMethodChecked(const std::string& name,
                           const std::vector<lcs::ScriptValue>& args,
                           lcs::ScriptValue& result,
                           std::string& error) override {
        py::gil_scoped_acquire gil;
        result = lcs::ScriptValue{};
        error.clear();
        try {
            if (!py::hasattr(_obj, name.c_str())) return true;
            py::object method = _obj.attr(name.c_str());
            py::tuple pyArgs(args.size());
            for (std::size_t i = 0; i < args.size(); ++i) {
                pyArgs[i] = lc::python::toPy(args[i]);
            }
            result = lc::python::fromPy(method(*pyArgs));
            return true;
        } catch (py::error_already_set& exception) {
            error = exception.what();
        } catch (const std::exception& exception) {
            error = exception.what();
        } catch (...) {
            error = "Python method failed with an unknown exception";
        }
        return false;
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
            pyArgs[i] = lc::python::toPy(args[i]);
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
