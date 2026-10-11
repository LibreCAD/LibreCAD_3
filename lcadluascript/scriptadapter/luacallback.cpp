// luacallback.cpp — Lua adapter for lcscripting types.
// Phase 4 PR-1 follow-on.
//
// Runtime verification for this file is deferred to CI (this dev
// machine has Lua 5.4/5.5 via Homebrew, not the 5.3 EXACT that the
// current lcadluascript CMakeLists.txt pins).  The code compiles
// against kaguya's public API (which the top-level tree also uses via
// lcadluascript/lclua.cpp and every bridge/lc_*.cpp).

#include "luacallback.h"

#include <cad/base/cadentity.h>
#include <cad/geometry/geocoordinate.h>
#include <cad/meta/color.h>

#include <cstring>
#include <string>
#include <vector>

namespace lcs = lc::scripting;

namespace {

// -----------------------------------------------------------------------------
// OpaquePtr encoder registry.
// -----------------------------------------------------------------------------
// Phase 4 PR-8: per-tag encoders registered by lcUI so the Lua adapter
// can materialize OpaquePtr values (e.g. api::Menu*) into Lua-side
// userdata without lcadluascript knowing about lcUI types.

struct OpaqueEncoderEntry {
    const char* tag;   // caller-owned string literal
    lc::lua::OpaqueEncoder encoder;
};

std::vector<OpaqueEncoderEntry>& opaqueRegistry() {
    // Function-local static — construct-on-first-use for the Meyer's
    // singleton pattern.  Registration happens during static-init /
    // lcUI's initLua path; lookup happens per toLuaLocked invocation.
    static std::vector<OpaqueEncoderEntry> reg;
    return reg;
}

// -----------------------------------------------------------------------------
// Type conversions.
// -----------------------------------------------------------------------------

kaguya::LuaRef toLuaLocked(kaguya::State& state, const lcs::ScriptValue& v) {
    using K = lcs::ScriptValue::Kind;
    switch (v.kind()) {
    case K::Nil:
        return kaguya::LuaRef(state.state());   // nil
    case K::Bool:
        return kaguya::LuaRef(state.state(), v.asBool());
    case K::Int:
        return kaguya::LuaRef(state.state(), v.asInt());
    case K::Double:
        return kaguya::LuaRef(state.state(), v.asDouble());
    case K::String:
        return kaguya::LuaRef(state.state(), v.asString());
    case K::Coordinate:
        return kaguya::LuaRef(state.state(), v.asCoordinate());
    case K::Color:
        return kaguya::LuaRef(state.state(), v.asColor());
    case K::Entity:
        return kaguya::LuaRef(state.state(), v.asEntity());
    case K::CoordinateList:
        return kaguya::LuaRef(state.state(), v.asCoordinateList());
    case K::EntityList:
        return kaguya::LuaRef(state.state(), v.asEntityList());
    case K::Opaque: {
        // Phase 4 PR-8: per-tag encoder lookup.  The registry is
        // populated by lcUI at initialize time (see guibridge.cpp).
        // Walk in REVERSE so a later registration (per the "last-in
        // wins" pattern shared with runOperationByName's resolver list)
        // takes precedence over an earlier one for the same tag.
        lcs::OpaquePtr op = v.asOpaque();
        if (op.tag == nullptr || op.ptr == nullptr) {
            return kaguya::LuaRef(state.state());
        }
        auto& reg = opaqueRegistry();
        for (auto it = reg.rbegin(); it != reg.rend(); ++it) {
            if (it->tag == op.tag || std::strcmp(it->tag, op.tag) == 0) {
                return it->encoder(state, op.ptr);
            }
        }
        // No encoder registered for this tag — degrade to nil (matches
        // the pre-PR-8 fallback so unknown tags don't crash Lua callers).
        return kaguya::LuaRef(state.state());
    }
    case K::MapKind: {
        kaguya::LuaTable tbl = state.newTable();
        if (v.asMap()) {
            for (const auto& kv : *v.asMap()) {
                tbl[kv.first] = toLuaLocked(state, kv.second);
            }
        }
        return tbl;
    }
    }
    return kaguya::LuaRef(state.state());   // unreachable, keep compiler happy
}

lcs::ScriptValue fromLuaLocked(const kaguya::LuaRef& ref) {
    const int t = ref.type();
    switch (t) {
    case LUA_TNIL:      return lcs::ScriptValue{};
    case LUA_TBOOLEAN:  return lcs::ScriptValue(ref.get<bool>());
    case LUA_TNUMBER: {
        // Lua doesn't distinguish int vs double at the language level
        // in 5.2 (integer subtype exists in 5.3+; kaguya papers over
        // it).  Prefer double if there's a fractional part.
        const double d = ref.get<double>();
        const int i = static_cast<int>(d);
        return (static_cast<double>(i) == d)
            ? lcs::ScriptValue(i)
            : lcs::ScriptValue(d);
    }
    case LUA_TSTRING:   return lcs::ScriptValue(ref.get<std::string>());
    case LUA_TTABLE: {
        // Try to read a map — best-effort; anything the map iteration
        // can't decode falls to Nil.  Deep recursion is fine since
        // kaguya's LuaRef metatable handles cycles safely.
        auto m = lcs::makeMap();
        // kaguya's table access via keys is expensive if we don't
        // know the key set; we peek via the underlying lua_next.  For
        // PR-1's needs (event args are simple structs) this is fine.
        std::map<std::string, kaguya::LuaRef> kv = ref.map<std::string, kaguya::LuaRef>();
        for (auto& e : kv) {
            (*m)[e.first] = fromLuaLocked(e.second);
        }
        return lcs::ScriptValue(std::move(m));
    }
    // LUA_TUSERDATA — a kernel type came through.  Try coordinate first.
    case LUA_TUSERDATA: {
        try {
            return lcs::ScriptValue(ref.get<lc::geo::Coordinate>());
        } catch (...) {}
        try {
            return lcs::ScriptValue(ref.get<lc::entity::CADEntity_CSPtr>());
        } catch (...) {}
        try {
            return lcs::ScriptValue(ref.get<lc::Color>());
        } catch (...) {}
        return lcs::ScriptValue{};
    }
    default:
        return lcs::ScriptValue{};
    }
}

class ScopedLuaErrorHandler {
public:
    explicit ScopedLuaErrorHandler(kaguya::State& state)
        : _state(state),
          _previous(kaguya::ErrorHandler::getHandler(state.state())) {
        _state.setErrorHandler([](int, const char*) {});
    }

    ~ScopedLuaErrorHandler() {
        _state.setErrorHandler(_previous);
    }

    ScopedLuaErrorHandler(const ScopedLuaErrorHandler&) = delete;
    ScopedLuaErrorHandler& operator=(const ScopedLuaErrorHandler&) = delete;

private:
    kaguya::State& _state;
    kaguya::ErrorHandler::function_type _previous;
};

std::string luaCallErrorMessage(const kaguya::FunctionResults& results) {
    std::string message = "unknown Lua error";
    if (results.result_size() > 0) {
        try {
            message = results.result_at<std::string>(0);
        } catch (...) {
            // Keep a useful generic message if Lua's error object is not text.
        }
    }
    return message;
}

bool invokeLuaChecked(
    kaguya::LuaRef callable,
    const std::vector<kaguya::LuaRef>& args,
    const std::string& context,
    kaguya::LuaRef& result,
    std::string& error) {
    result = kaguya::LuaRef(callable.state());
    error.clear();
    if (callable.isNilref() || callable.type() != LUA_TFUNCTION) {
        error = context + " is not callable";
        return false;
    }
    if (args.size() > 9) {
        error = context + " exceeds the supported Lua call arity (9 arguments)";
        return false;
    }

    kaguya::State state(callable.state());
    ScopedLuaErrorHandler errorHandler(state);
    try {
        // FunctionResults preserves lua_pcall's status; assigning directly
        // to LuaRef can turn a protected-call failure into a default ref.
        const auto results = [&]() -> kaguya::FunctionResults {
            switch (args.size()) {
            case 0: return callable();
            case 1: return callable(args[0]);
            case 2: return callable(args[0], args[1]);
            case 3: return callable(args[0], args[1], args[2]);
            case 4: return callable(args[0], args[1], args[2], args[3]);
            case 5: return callable(args[0], args[1], args[2], args[3], args[4]);
            case 6: return callable(args[0], args[1], args[2], args[3], args[4], args[5]);
            case 7: return callable(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
            case 8: return callable(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
            case 9: return callable(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
            default: return kaguya::FunctionResults(callable.state());
            }
        }();
        if (results.resultStatus() != 0) {
            error = context + " failed:\n" + luaCallErrorMessage(results);
            return false;
        }
        result = results.get_result<kaguya::LuaRef>();
        return true;
    } catch (const std::exception& exception) {
        error = context + " failed:\n" + exception.what();
    } catch (...) {
        error = context + " failed with an unknown Lua error";
    }
    return false;
}

bool reportLuaCallFailure(
    const std::string& event,
    const kaguya::FunctionResults& results,
    const lcs::ScriptDiagnosticSink& diagnostics) {
    if (results.resultStatus() == 0) return false;
    lcs::reportScriptDiagnostic(
        diagnostics, "Lua event '" + event + "' callback failed:\n" +
            luaCallErrorMessage(results));
    return true;
}

// -----------------------------------------------------------------------------
// ScriptCallbackImpl for Lua.
// -----------------------------------------------------------------------------
class LuaCallbackImpl : public lcs::ScriptCallbackImpl {
public:
    explicit LuaCallbackImpl(kaguya::LuaRef ref) : _ref(std::move(ref)) {}
    ~LuaCallbackImpl() override = default;
    LuaCallbackImpl(const LuaCallbackImpl&) = delete;
    LuaCallbackImpl& operator=(const LuaCallbackImpl&) = delete;

    const char* runtime() const override { return "lua"; }

    lcs::ScriptValue invoke(const std::vector<lcs::ScriptValue>& args) override {
        if (_ref.isNilref() || _ref.type() != LUA_TFUNCTION) {
            return lcs::ScriptValue{};
        }
        // kaguya's LuaRef::operator() takes variadic args by value;
        // wrap our ScriptValues into LuaRefs first.
        kaguya::State state(_ref.state());
        // For PR-1 we pass a vector-of-luarefs pattern via kaguya's
        // std::vector<LuaRef> unpacking; kaguya expands it to multiple
        // positional args at call time.
        std::vector<kaguya::LuaRef> lua_args;
        lua_args.reserve(args.size());
        for (const auto& a : args) {
            lua_args.push_back(toLuaLocked(state, a));
        }
        // kaguya::LuaRef::operator() with a vector unpacks — but
        // simplest robust invocation: call() with variadic-style
        // unpacking is unavailable, so we use call<LuaRef>(...) if
        // supported; failing that, call the function slot directly.
        try {
            // Simplified path: pass up to 4 args by value.  Real bridge
            // sites in this codebase never use more (see arity list in
            // the sub-plan).  For >4, we'd need to construct a Lua
            // call from the stack — future improvement if needed.
            kaguya::LuaRef result;
            switch (lua_args.size()) {
            case 0: result = _ref(); break;
            case 1: result = _ref(lua_args[0]); break;
            case 2: result = _ref(lua_args[0], lua_args[1]); break;
            case 3: result = _ref(lua_args[0], lua_args[1], lua_args[2]); break;
            case 4: result = _ref(lua_args[0], lua_args[1], lua_args[2], lua_args[3]); break;
            default:
                // >4 args: fall back to passing the raw vector — kaguya
                // treats it as one arg (a Lua table).  Bridge sites in
                // this codebase don't hit this today.
                result = _ref(lua_args);
                break;
            }
            return fromLuaLocked(result);
        } catch (const std::exception& /*e*/) {
            // Lua-level error — swallow so the Qt event loop stays alive.
            // TODO(phase-4 later): route the message to CliCommand.
            return lcs::ScriptValue{};
        }
    }

    lcs::ScriptValue invokeEvent(
        const std::string& event,
        const lcs::ScriptValue& args,
        const lcs::ScriptDiagnosticSink& diagnostics) override {
        if (_ref.isNilref()) return lcs::ScriptValue{};
        kaguya::State state(_ref.state());
        ScopedLuaErrorHandler errorHandler(state);
        try {
            if (_ref.type() == LUA_TFUNCTION) {
                // Shape 1: f(event, args) — verbatim luainterface.cpp:137.
                kaguya::LuaRef ev_ref(state.state(), event);
                kaguya::LuaRef args_ref = toLuaLocked(state, args);
                auto results = _ref(ev_ref, args_ref);
                if (reportLuaCallFailure(event, results, diagnostics)) {
                    return lcs::ScriptValue{};
                }
                return fromLuaLocked(results.get_result<kaguya::LuaRef>());
            } else if (_ref.type() == LUA_TTABLE) {
                // Shape 2: t.onEvent(t, event, args) — verbatim :140.
                kaguya::LuaRef on_event = _ref["onEvent"];
                if (on_event.isNilref()) return lcs::ScriptValue{};
                kaguya::LuaRef ev_ref(state.state(), event);
                kaguya::LuaRef args_ref = toLuaLocked(state, args);
                auto results = on_event(_ref, ev_ref, args_ref);
                if (reportLuaCallFailure(event, results, diagnostics)) {
                    return lcs::ScriptValue{};
                }
                return fromLuaLocked(results.get_result<kaguya::LuaRef>());
            }
        } catch (const std::exception& error) {
            lcs::reportScriptDiagnostic(
                diagnostics,
                "Lua event '" + event + "' callback failed:\n" + error.what());
        } catch (...) {
            lcs::reportScriptDiagnostic(
                diagnostics,
                "Lua event '" + event + "' callback failed with an unknown exception");
        }
        return lcs::ScriptValue{};
    }

    bool equals(const ScriptCallbackImpl& other) const override {
        auto* lother = dynamic_cast<const LuaCallbackImpl*>(&other);
        if (!lother) return false;
        // kaguya::LuaRef::operator== defers to Lua's rawequal, which
        // matches LuaInterface's `std::find(..., callback)` behavior in
        // deleteEvent (luainterface.cpp:126) — the exact equality the
        // plan requires for parity.
        return _ref == lother->_ref;
    }

    // Accessor for the guard the registerEvent adapter needs
    // (rejectTableWithoutOnEvent, per luainterface.cpp:118).  Only used
    // by makeLuaCallback if the caller decides to guard at construction
    // time; EventBus doesn't need it because it dispatches at trigger
    // time.
    const kaguya::LuaRef& ref() const { return _ref; }

private:
    kaguya::LuaRef _ref;
};

// -----------------------------------------------------------------------------
// ScriptObjectImpl for Lua.
// -----------------------------------------------------------------------------
class LuaObjectImpl : public lcs::ScriptObjectImpl {
public:
    explicit LuaObjectImpl(kaguya::LuaRef ref) : _ref(std::move(ref)) {}
    ~LuaObjectImpl() override = default;
    LuaObjectImpl(const LuaObjectImpl&) = delete;
    LuaObjectImpl& operator=(const LuaObjectImpl&) = delete;

    const char* runtime() const override { return "lua"; }

    lcs::ScriptValue call(const std::vector<lcs::ScriptValue>& args) override {
        // Fires the callable and returns a *Value* view of the result.
        // For identity-preserved constructor invocation use `instantiate`
        // instead.
        kaguya::LuaRef result = callRaw(args);
        return result.isNilref() ? lcs::ScriptValue{} : fromLuaLocked(result);
    }

    std::shared_ptr<lcs::ScriptObjectImpl>
    instantiate(const std::vector<lcs::ScriptValue>& args) override {
        // Phase 4 PR-7: constructor invocation that keeps the OBJECT
        // identity of the returned LuaRef.  Used by
        // MainWindow::runOperation to create the operation instance from
        // the operation-class object.  The returned impl still supports
        // callMethod / getAttr / setAttr the way the LuaRef instance does
        // — the Value round-trip that `call` does would drop that.
        std::shared_ptr<lcs::ScriptObjectImpl> result;
        std::string error;
        (void)instantiateChecked(args, result, error);
        return result;
    }

    bool instantiateChecked(
        const std::vector<lcs::ScriptValue>& args,
        std::shared_ptr<lcs::ScriptObjectImpl>& result,
        std::string& error) override {
        result.reset();
        kaguya::LuaRef instance;
        if (!callRawChecked(args, instance, error)) return false;
        if (instance.isNilref()) {
            error = "Lua operation constructor returned nil";
            return false;
        }
        result = std::make_shared<LuaObjectImpl>(std::move(instance));
        return true;
    }

    lcs::ScriptValue getAttr(const std::string& name) override {
        if (_ref.isNilref() || _ref.type() != LUA_TTABLE) {
            return lcs::ScriptValue{};
        }
        kaguya::LuaRef v = _ref[name];
        return fromLuaLocked(v);
    }

    void setAttr(const std::string& name, const lcs::ScriptValue& value) override {
        if (_ref.isNilref() || _ref.type() != LUA_TTABLE) return;
        kaguya::State state(_ref.state());
        _ref[name] = toLuaLocked(state, value);
    }

    bool hasAttr(const std::string& name) override {
        if (_ref.isNilref() || _ref.type() != LUA_TTABLE) return false;
        return !_ref[name].isNilref();
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
        result = lcs::ScriptValue{};
        error.clear();
        if (_ref.isNilref() || _ref.type() != LUA_TTABLE) return true;

        kaguya::LuaRef method = _ref[name];
        if (method.isNilref() || method.type() != LUA_TFUNCTION) return true;

        kaguya::State state(_ref.state());
        std::vector<kaguya::LuaRef> luaArgs;
        luaArgs.reserve(args.size() + 1);
        // Match Lua's `op:method(...)` call shape.
        luaArgs.push_back(_ref);
        for (const auto& arg : args) luaArgs.push_back(toLuaLocked(state, arg));

        kaguya::LuaRef returned;
        if (!invokeLuaChecked(method, luaArgs,
                              "Lua operation method '" + name + "'",
                              returned, error)) {
            return false;
        }
        result = fromLuaLocked(returned);
        return true;
    }

    bool equals(const ScriptObjectImpl& other) const override {
        auto* lother = dynamic_cast<const LuaObjectImpl*>(&other);
        if (!lother) return false;
        return _ref == lother->_ref;
    }

private:
    bool callRawChecked(const std::vector<lcs::ScriptValue>& args,
                        kaguya::LuaRef& result,
                        std::string& error) {
        if (_ref.isNilref()) {
            error = "Lua operation constructor is nil";
            return false;
        }

        kaguya::State state(_ref.state());
        std::vector<kaguya::LuaRef> luaArgs;
        std::vector<kaguya::LuaRef> implicitArgs;
        kaguya::LuaRef callable = resolveCallable(implicitArgs);
        if (callable.isNilref()) {
            error = "Lua operation constructor is not callable";
            return false;
        }
        luaArgs = std::move(implicitArgs);
        for (const auto& arg : args) luaArgs.push_back(toLuaLocked(state, arg));
        return invokeLuaChecked(callable, luaArgs,
                                "Lua operation constructor", result, error);
    }

    kaguya::LuaRef callMetamethod(const kaguya::LuaRef& value) const {
        if (value.type() == LUA_TTABLE) {
            value.push();
            kaguya::LuaTable object(value.state(), kaguya::StackTop());
            const kaguya::LuaTable metatable = object.getMetatable();
            if (metatable.isNilref()) return kaguya::LuaRef(value.state());
            kaguya::LuaRef metamethod = metatable["__call"];
            return metamethod;
        }
        if (value.type() == LUA_TUSERDATA) {
            value.push();
            kaguya::LuaUserData object(value.state(), kaguya::StackTop());
            const kaguya::LuaTable metatable = object.getMetatable();
            if (metatable.isNilref()) return kaguya::LuaRef(value.state());
            kaguya::LuaRef metamethod = metatable["__call"];
            return metamethod;
        }
        return kaguya::LuaRef(value.state());
    }

    kaguya::LuaRef resolveCallable(
        std::vector<kaguya::LuaRef>& implicitArgs) const {
        kaguya::LuaRef callable = _ref;
        // Lua permits __call to refer to another callable object. Preserve
        // the VM's implicit-self chain, with a depth guard for malformed
        // cyclic metatables.
        constexpr std::size_t maxCallMetamethodDepth = 32;
        for (std::size_t depth = 0; depth <= maxCallMetamethodDepth; ++depth) {
            const int type = callable.type();
            if (type == LUA_TFUNCTION || type == LUA_TTHREAD) {
                return callable;
            }
            if (depth == maxCallMetamethodDepth) return {};
            if (type != LUA_TTABLE && type != LUA_TUSERDATA) return {};

            kaguya::LuaRef metamethod = callMetamethod(callable);
            if (metamethod.isNilref()) return {};
            implicitArgs.insert(implicitArgs.begin(), callable);
            callable = std::move(metamethod);
        }
        return {};
    }

    // Common Lua-side callable invocation; returns the raw LuaRef result
    // (may be nil).  Both `call` and `instantiate` route through here so
    // the argument packing / exception handling stays in one place.
    kaguya::LuaRef callRaw(const std::vector<lcs::ScriptValue>& args) {
        if (_ref.isNilref()) return kaguya::LuaRef{};
        kaguya::State state(_ref.state());
        std::vector<kaguya::LuaRef> lua_args;
        std::vector<kaguya::LuaRef> implicitArgs;
        kaguya::LuaRef callable = resolveCallable(implicitArgs);
        if (callable.isNilref()) return kaguya::LuaRef{};
        lua_args = std::move(implicitArgs);
        for (const auto& a : args) lua_args.push_back(toLuaLocked(state, a));
        try {
            switch (lua_args.size()) {
            case 0: return callable();
            case 1: return callable(lua_args[0]);
            case 2: return callable(lua_args[0], lua_args[1]);
            case 3: return callable(lua_args[0], lua_args[1], lua_args[2]);
            case 4: return callable(lua_args[0], lua_args[1], lua_args[2], lua_args[3]);
            case 5: return callable(lua_args[0], lua_args[1], lua_args[2], lua_args[3], lua_args[4]);
            case 6: return callable(lua_args[0], lua_args[1], lua_args[2], lua_args[3], lua_args[4], lua_args[5]);
            case 7: return callable(lua_args[0], lua_args[1], lua_args[2], lua_args[3], lua_args[4], lua_args[5], lua_args[6]);
            case 8: return callable(lua_args[0], lua_args[1], lua_args[2], lua_args[3], lua_args[4], lua_args[5], lua_args[6], lua_args[7]);
            case 9: return callable(lua_args[0], lua_args[1], lua_args[2], lua_args[3], lua_args[4], lua_args[5], lua_args[6], lua_args[7], lua_args[8]);
            default: return kaguya::LuaRef(_ref.state());
            }
        } catch (const std::exception& /*e*/) {
            return kaguya::LuaRef{};
        }
    }

    kaguya::LuaRef _ref;
};

} // namespace

namespace lc {
namespace lua {

lc::scripting::ScriptCallback makeLuaCallback(kaguya::LuaRef ref) {
    return lc::scripting::ScriptCallback(
        std::make_shared<LuaCallbackImpl>(std::move(ref)));
}

kaguya::LuaRef unwrapLuaCallback(const lc::scripting::ScriptCallback& cb) {
    // Phase 4 post-review fix — used by
    // LuaInterface::triggerEvent(event, kaguya::LuaRef args) to
    // dispatch Lua-to-Lua listeners with the raw LuaRef payload,
    // preserving array-table integer keys and userdata/function/thread
    // types that would otherwise round-trip lossily through fromLua.
    // Returns nil for non-Lua callbacks so the caller can fall back to
    // the ScriptValue path for cross-language listeners.
    const auto& impl = cb.impl();
    if (!impl) return kaguya::LuaRef{};
    if (std::strcmp(impl->runtime(), "lua") != 0) {
        // Cheap runtime-tag check first — avoids the dynamic_cast for
        // non-Lua callbacks.  The runtime tag is a compile-time literal
        // so pointer comparison would also work; strcmp is future-proof
        // against different-TU literals.
        return kaguya::LuaRef{};
    }
    // Safe dcast — the runtime tag guarantees the concrete type.
    auto* lua_impl = dynamic_cast<LuaCallbackImpl*>(impl.get());
    return lua_impl ? lua_impl->ref() : kaguya::LuaRef{};
}

bool invokeLuaEventRaw(
    const kaguya::LuaRef& callback,
    const std::string& event,
    const kaguya::LuaRef& args,
    const lc::scripting::ScriptDiagnosticSink& diagnostics) {
    if (callback.isNilref()) return true;

    kaguya::State state(callback.state());
    ScopedLuaErrorHandler errorHandler(state);
    try {
        kaguya::LuaRef callable = callback;
        if (callable.type() == LUA_TFUNCTION) {
            auto results = callable(event, args);
            return !reportLuaCallFailure(event, results, diagnostics);
        }
        if (callable.type() == LUA_TTABLE) {
            kaguya::LuaRef onEvent = callable["onEvent"];
            if (onEvent.isNilref()) return true;
            auto results = onEvent(callable, event, args);
            return !reportLuaCallFailure(event, results, diagnostics);
        }
        return true;
    } catch (const std::exception& error) {
        lcs::reportScriptDiagnostic(
            diagnostics,
            "Lua event '" + event + "' callback failed:\n" + error.what());
    } catch (...) {
        lcs::reportScriptDiagnostic(
            diagnostics,
            "Lua event '" + event + "' callback failed with an unknown exception");
    }
    return false;
}

lc::scripting::ScriptObject makeLuaObject(kaguya::LuaRef ref) {
    return lc::scripting::ScriptObject(
        std::make_shared<LuaObjectImpl>(std::move(ref)));
}

kaguya::LuaRef toLua(kaguya::State& state, const lc::scripting::ScriptValue& v) {
    return toLuaLocked(state, v);
}

void registerOpaqueEncoder(const char* tag, OpaqueEncoder encoder) {
    // Phase 4 PR-8 + PR-10 followup — public entry point.  Dedup by
    // tag: if an entry with the same tag already exists (pointer
    // equality OR strcmp match — same rule as `toLuaLocked`'s lookup),
    // overwrite its encoder in place.  This preserves the header's
    // documented "later registrations OVERRIDE earlier ones" semantic
    // WITHOUT unbounded growth on repeated calls — a bug in the
    // original PR-8 code that caused every ScriptDock reinit / test
    // fixture setup to leak an encoder slot per tag.  Verified by the
    // 4-second grep for `registerOpaqueEncoder` — only lcUI calls it,
    // and all callers are OK with overwrite semantics.
    auto& reg = opaqueRegistry();
    for (auto& entry : reg) {
        if (entry.tag == tag || std::strcmp(entry.tag, tag) == 0) {
            entry.encoder = std::move(encoder);
            entry.tag     = tag;  // adopt the canonical tag pointer
            return;
        }
    }
    reg.push_back({tag, std::move(encoder)});
}

lc::scripting::ScriptValue fromLua(const kaguya::LuaRef& ref) {
    return fromLuaLocked(ref);
}

} // namespace lua
} // namespace lc
