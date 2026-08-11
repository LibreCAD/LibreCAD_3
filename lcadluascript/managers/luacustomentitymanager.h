#pragma once

#include <lclua.h>
#include <kaguya/kaguya.hpp>

// Phase 6 PR-6.1 — LuaCustomEntityManager neutralized to store
// ScriptCallback plugin slots instead of raw kaguya::LuaRef.  The Lua
// side wraps its LuaRef via makeLuaCallback at the binding site
// (lclua.cpp); a new lcadpythonscript-side binding wraps py::object via
// makePythonCallback.  Both languages share the same singleton manager.
//
// The class KEEPS its historical name (LuaCustomEntityManager) for
// caller compatibility during phase 6; a full rename to
// `CustomEntityManager` in `lcscripting/` is scoped for a later
// sub-piece.  The neutralization here is the surgical prerequisite:
// once the manager stores neutral callbacks, moving it becomes purely
// a filesystem/CMake change without touching call sites.
#include <lcscripting/scriptcallback.h>

namespace lc {
namespace lua {
class LuaCustomEntityManager {
public:
    static LuaCustomEntityManager& getInstance() {
        static LuaCustomEntityManager _instance;

        return _instance;
    }

    LuaCustomEntityManager(LuaCustomEntityManager const&) = delete;

    void operator=(LuaCustomEntityManager const&) = delete;

    virtual ~LuaCustomEntityManager();

    /**
     * @brief Register a new plugin which handle custom entities (LuaRef overload).
     * @param name Name of the plugin
     * @param onNewWaitingEntityFunction Function called when there are entities which needs to be recreated by the plugin
     *
     * Phase 6 PR-6.1 — this overload wraps the LuaRef via
     * `lc::lua::makeLuaCallback` internally and forwards to the neutral
     * `registerPlugin(string, ScriptCallback)`.  Kaguya callers see zero
     * behavior change.
     */
    void registerPlugin(const std::string& name, kaguya::LuaRef onNewWaitingEntityFunction);

    /**
     * @brief Register a new plugin using a neutral ScriptCallback.
     * @param name Name of the plugin
     * @param callback Neutral callback invoked with the Insert_CSPtr
     *                 (packaged as ScriptValue(Kind::Entity)) when a
     *                 waiting custom entity matches @p name.
     *
     * Phase 6 PR-6.1 — this is the target entry point.  Both the Lua
     * and the Python adapter wrap their language-specific callable
     * shape into a ScriptCallback and call this.  Also usable by
     * headless C++ tests via `lc::scripting::nativeCallback(...)` —
     * see pythonbindings_test.cpp's regression guards.
     */
    void registerPlugin(const std::string& name,
                        lc::scripting::ScriptCallback callback);

    /**
     * @brief Remove all registered plugins.
     *
     * Phase 6 PR-6.1 — this is process-scoped teardown (call at
     * process exit only).  The MULTI-WINDOW BUG fix: `~LuaInterface`
     * previously called this on the shared singleton, which cleared
     * plugins for ALL windows when ANY single window closed.  The
     * fix is to NOT call this from per-window destructors — the
     * singleton's own destructor at process exit is the only
     * legitimate teardown point.
     */
    void removePlugins();

    /**
     * @brief Query whether a plugin is registered under @p name.
     *
     * Phase 6 PR-6.1 — added for testability.  Headless tests use this
     * to prove `registerPlugin` actually stored the plugin (without
     * needing to fire a NewWaitingCustomEntityEvent, which requires
     * a fully-constructed Insert with a CustomEntityStorage displayBlock).
     */
    bool hasPlugin(const std::string& name) const;

private:
    LuaCustomEntityManager();

    void onNewWaitingEntity(const lc::event::NewWaitingCustomEntityEvent& event);

    // Phase 6 PR-6.1 — was `std::map<std::string, kaguya::LuaRef>`;
    // now stores the neutral ScriptCallback.  Both Lua and Python
    // adapters wrap their callable into a ScriptCallback at the
    // binding site; the manager itself no longer knows about either
    // language runtime.
    std::map<std::string, lc::scripting::ScriptCallback> _plugins;
};
}
}
