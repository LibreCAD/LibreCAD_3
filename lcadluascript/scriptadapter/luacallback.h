#pragma once

// Lua adapter for lcscripting::ScriptCallback and ScriptObject.
// Phase 4 PR-1 follow-on.
//
// Wraps `kaguya::LuaRef` in the neutral callback / object types so
// lcUI can hold Lua callbacks/instances behind the language-neutral
// interface.  Behavior mirrors LuaInterface's existing shape verbatim
// (that's the "Lua semantics unchanged" hard constraint):
//   * registerEvent guard: if the LuaRef is a table without an
//     `onEvent` field, reject it (matches luainterface.cpp:118).
//   * triggerEvent shape dispatch (luainterface.cpp:133-143):
//       - LUA_TFUNCTION → f(event, args)
//       - LUA_TTABLE    → t.onEvent(t, event, args)
//   * equality via `kaguya::LuaRef::operator==` (which delegates to Lua's
//     `lua_rawequal` — same reference or metamethod-agnostic equality).
//
// Concurrency: Lua does NOT have a Python-style GIL; concurrent access
// to the same lua_State from multiple threads is UB.  For PR-1 we
// assume the caller (MainWindow / ScriptDock / luacmdinterface) uses
// its own lua_State on its own thread — matches today's behavior.  A
// mutex here would be pessimization for the current single-thread use.

#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptobject.h>
#include <lcscripting/scriptvalue.h>

#include <kaguya/kaguya.hpp>
#include <functional>

namespace lc {
namespace lua {

/// Per-tag encoder for OpaquePtr values.  lcUI registers encoders for
/// its own opaque types (api::Menu*, CadMdiChild*, ...) — lcscripting +
/// lcadluascript can't name them because they sit below lcUI.  The
/// registered encoder receives the raw void* payload and produces a
/// kaguya::LuaRef that Lua-side callbacks can use (typically by wrapping
/// the pointer as userdata via one of kaguya's UserdataMetatable bindings).
/// Phase 4 PR-8 add.
using OpaqueEncoder =
    std::function<kaguya::LuaRef(kaguya::State&, void*)>;

/// Register an encoder for the given tag.  Later registrations OVERRIDE
/// earlier ones for the same tag (matches the resolver-list "later wins"
/// pattern used in MainWindow::runOperationByName).  The registry
/// dedups on tag: a second call with the same tag overwrites the
/// encoder IN PLACE rather than appending — preserves "later wins"
/// while bounding memory to one entry per tag (see luacallback.cpp for
/// the PR-10-followup rationale — the original PR-8 code appended
/// unconditionally, leaking a slot per re-registration).  Tags are
/// compared by pointer identity first, then strcmp, so multiple TUs
/// can define the same tag as a bespoke `static const char*` literal.
void registerOpaqueEncoder(const char* tag, OpaqueEncoder encoder);

/// Wrap a kaguya::LuaRef in a ScriptCallback.  The callback's runtime()
/// tag is "lua".
lc::scripting::ScriptCallback makeLuaCallback(kaguya::LuaRef ref);

/// Wrap a kaguya::LuaRef (typically a table representing an operation
/// instance) in a ScriptObject.
lc::scripting::ScriptObject makeLuaObject(kaguya::LuaRef ref);

/// Convert a ScriptValue to a kaguya::LuaRef.  Uses the LuaRef's state
/// as the target Lua state.  Returns a nil LuaRef for OpaquePtr (the
/// UI-side materializer registers a per-tag encoder in later phase-4
/// PRs).
kaguya::LuaRef toLua(kaguya::State& state, const lc::scripting::ScriptValue& v);

/// Convert a kaguya::LuaRef to a ScriptValue.  Best-effort; falls back
/// to Nil for shapes the adapter can't map.
lc::scripting::ScriptValue fromLua(const kaguya::LuaRef& ref);

} // namespace lua
} // namespace lc
