// scriptcallback_native.cpp — the std::function adapter for ScriptCallback.
// Phase 4 PR-1.  Used as the replacement for dostring-codegen call sites
// (menu items, snap-op lambdas, ...) that don't need to route through a
// scripting-language runtime.

#include "scriptcallback.h"

#include <iostream>

namespace lc {
namespace scripting {

void reportScriptDiagnostic(const ScriptDiagnosticSink& sink,
                            const std::string& message) noexcept {
    try {
        if (sink) {
            sink(message);
        } else {
            std::cerr << message << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << "Script diagnostic sink failed: " << error.what()
                  << "\n" << message << '\n';
    } catch (...) {
        std::cerr << "Script diagnostic sink failed with an unknown exception\n"
                  << message << '\n';
    }
}

namespace {

/// Base for the two native shapes.  Equality on native callbacks is
/// pointer-of-pImpl only — std::function has no stable identity, and the
/// use cases that construct native callbacks (menu items, code-gen
/// replacements) never need "same target" equality; they need "same
/// registration" equality, which pImpl identity provides.
class NativeImplBase : public ScriptCallbackImpl {
public:
    const char* runtime() const override { return "native"; }
    bool equals(const ScriptCallbackImpl& other) const override {
        return this == &other;
    }
    // Native "onEvent" shape: forward as `invoke({event_name, args})` so
    // the underlying std::function sees a 2-element vector [event, args].
    // This matches the "Lua function → f(event, args)" shape and lets
    // EventBus fire native callbacks the same way as language callbacks.
    // Subclasses implement invoke().
    ScriptValue invokeEvent(const std::string& event,
                            const ScriptValue& args,
                            const ScriptDiagnosticSink& /*diagnostics*/) override {
        return invoke({ScriptValue(event), args});
    }
};

class NativeVector : public NativeImplBase {
public:
    explicit NativeVector(std::function<ScriptValue(const std::vector<ScriptValue>&)> fn)
        : _fn(std::move(fn)) {}
    ScriptValue invoke(const std::vector<ScriptValue>& args) override {
        return _fn ? _fn(args) : ScriptValue{};
    }
private:
    std::function<ScriptValue(const std::vector<ScriptValue>&)> _fn;
};

class NativeVoid : public NativeImplBase {
public:
    explicit NativeVoid(std::function<void()> fn) : _fn(std::move(fn)) {}
    ScriptValue invoke(const std::vector<ScriptValue>& /*args*/) override {
        if (_fn) _fn();
        return ScriptValue{};
    }
private:
    std::function<void()> _fn;
};

} // namespace

ScriptCallback nativeCallback(std::function<ScriptValue(const std::vector<ScriptValue>&)> fn) {
    return ScriptCallback(std::make_shared<NativeVector>(std::move(fn)));
}

ScriptCallback nativeCallback(std::function<void()> fn) {
    return ScriptCallback(std::make_shared<NativeVoid>(std::move(fn)));
}

} // namespace scripting
} // namespace lc
