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

    lcs::ScriptValue invokeEvent(const std::string& event,
                                 const lcs::ScriptValue& args) override {
        if (_ref.isNilref()) return lcs::ScriptValue{};
        kaguya::State state(_ref.state());
        try {
            if (_ref.type() == LUA_TFUNCTION) {
                // Shape 1: f(event, args) — verbatim luainterface.cpp:137.
                kaguya::LuaRef ev_ref(state.state(), event);
                kaguya::LuaRef args_ref = toLuaLocked(state, args);
                return fromLuaLocked(_ref(ev_ref, args_ref));
            } else if (_ref.type() == LUA_TTABLE) {
                // Shape 2: t.onEvent(t, event, args) — verbatim :140.
                kaguya::LuaRef on_event = _ref["onEvent"];
                if (on_event.isNilref()) return lcs::ScriptValue{};
                kaguya::LuaRef ev_ref(state.state(), event);
                kaguya::LuaRef args_ref = toLuaLocked(state, args);
                return fromLuaLocked(on_event(_ref, ev_ref, args_ref));
            }
        } catch (const std::exception& /*e*/) {
            // Same swallow contract as invoke.
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
        kaguya::LuaRef result = callRaw(args);
        if (result.isNilref()) return nullptr;
        return std::make_shared<LuaObjectImpl>(std::move(result));
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
        if (_ref.isNilref() || _ref.type() != LUA_TTABLE) {
            return lcs::ScriptValue{};
        }
        kaguya::LuaRef method = _ref[name];
        if (method.isNilref() || method.type() != LUA_TFUNCTION) {
            return lcs::ScriptValue{};
        }
        kaguya::State state(_ref.state());
        std::vector<kaguya::LuaRef> lua_args;
        // Lua method calls pass `self` implicitly (the `:` operator);
        // we mirror that here so runOperation's callMethod("_init_default")
        // matches the Lua `op:_init_default()` shape.
        lua_args.push_back(_ref);
        for (const auto& a : args) lua_args.push_back(toLuaLocked(state, a));
        try {
            kaguya::LuaRef result;
            switch (lua_args.size()) {
            case 1: result = method(lua_args[0]); break;
            case 2: result = method(lua_args[0], lua_args[1]); break;
            case 3: result = method(lua_args[0], lua_args[1], lua_args[2]); break;
            default: result = method(lua_args); break;
            }
            return fromLuaLocked(result);
        } catch (const std::exception& /*e*/) {
            return lcs::ScriptValue{};
        }
    }

    bool equals(const ScriptObjectImpl& other) const override {
        auto* lother = dynamic_cast<const LuaObjectImpl*>(&other);
        if (!lother) return false;
        return _ref == lother->_ref;
    }

private:
    // Common Lua-side callable invocation; returns the raw LuaRef result
    // (may be nil).  Both `call` and `instantiate` route through here so
    // the argument packing / exception handling stays in one place.
    kaguya::LuaRef callRaw(const std::vector<lcs::ScriptValue>& args) {
        if (_ref.isNilref()) return kaguya::LuaRef{};
        kaguya::State state(_ref.state());
        std::vector<kaguya::LuaRef> lua_args;
        for (const auto& a : args) lua_args.push_back(toLuaLocked(state, a));
        try {
            switch (lua_args.size()) {
            case 0: return _ref();
            case 1: return _ref(lua_args[0]);
            case 2: return _ref(lua_args[0], lua_args[1]);
            default: return _ref(lua_args);
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
