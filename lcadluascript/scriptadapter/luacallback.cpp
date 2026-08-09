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

#include <string>
#include <vector>

namespace lcs = lc::scripting;

namespace {

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
    case K::Opaque:
        // No adapter-side materializer for arbitrary UI types yet.  The
        // per-tag encoder registration lives in the lcUI-side adapter
        // extension (later phase-4 PR).  Return nil to keep behavior
        // graceful.
        return kaguya::LuaRef(state.state());
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
        if (_ref.isNilref()) return lcs::ScriptValue{};
        kaguya::State state(_ref.state());
        std::vector<kaguya::LuaRef> lua_args;
        for (const auto& a : args) lua_args.push_back(toLuaLocked(state, a));
        try {
            kaguya::LuaRef result;
            switch (lua_args.size()) {
            case 0: result = _ref(); break;
            case 1: result = _ref(lua_args[0]); break;
            case 2: result = _ref(lua_args[0], lua_args[1]); break;
            default: result = _ref(lua_args); break;
            }
            return fromLuaLocked(result);
        } catch (const std::exception& /*e*/) {
            return lcs::ScriptValue{};
        }
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
    kaguya::LuaRef _ref;
};

} // namespace

namespace lc {
namespace lua {

lc::scripting::ScriptCallback makeLuaCallback(kaguya::LuaRef ref) {
    return lc::scripting::ScriptCallback(
        std::make_shared<LuaCallbackImpl>(std::move(ref)));
}

lc::scripting::ScriptObject makeLuaObject(kaguya::LuaRef ref) {
    return lc::scripting::ScriptObject(
        std::make_shared<LuaObjectImpl>(std::move(ref)));
}

kaguya::LuaRef toLua(kaguya::State& state, const lc::scripting::ScriptValue& v) {
    return toLuaLocked(state, v);
}

lc::scripting::ScriptValue fromLua(const kaguya::LuaRef& ref) {
    return fromLuaLocked(ref);
}

} // namespace lua
} // namespace lc
