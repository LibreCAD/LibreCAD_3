#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <set>
#include <QMetaObject>
#include <QMetaMethod>
#include <QObject>
#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QtUiTools/QUiLoader>
#include <QCoreApplication>
#include <managers/pluginmanager.h>

#include <kaguya/kaguya.hpp>
#include "lua/guibridge.h"

// Phase 4 PR-7 — _operation stored as neutral ScriptObject.  Adapters
// materialize into the runtime instance (Lua LuaRef or Python object).
#include <lcscripting/scriptobject.h>

// Phase 4 PR-9a — event registry delegated to lcscripting::EventBus.
// LuaInterface keeps its public LuaRef-taking API for backward compat
// with Lua-facing callers (guibridge + tests); a native ScriptValue
// overload of triggerEvent is added for MainWindow's trigger* slots
// (PR-9b) which now build ScriptValue payloads directly.
#include <lcscripting/eventbus.h>

namespace lc {
namespace ui {
/**
 * \brief Lua initializer
 */
class LuaInterface {
public:
    /**
     * \brief Create Lua instance
     */
    LuaInterface();

    ~LuaInterface();

    /**
     * \brief Read and execute Lua files
     */
    void initLua(QMainWindow* mainWindow);

    /**
     * \brief Load Qt widget from .ui file
     * \param fileName full path to .ui file
     */
    static QWidget* loadUiFile(const char* fileName);

    /**
     * \brief Return a list of plugins
     * \param path Path of plugins
     * \return List of strings containing the name of plugins
     */
    std::vector<std::string> pluginList(const char* path);

    /**
     * \brief Hide the window.
     * It needs to be used before initLua(), this is used in unit tests.
     */
    void hideUI(bool hidden);

    /**
     * \brief Returns current Lua state.
     * This is used for unit tests.
     */
    lua_State* luaState();

    static FILE* openFileDialog(bool isOpening, const char* description, const char* mode);

    /**
     * \brief Return the current operation instance (as a neutral
     * ScriptObject).  Phase 4 PR-7: was `kaguya::LuaRef`; adapters wrap
     * the underlying runtime object so the type stays language-neutral.
     */
    lc::scripting::ScriptObject operation();

    /**
     * \brief Set the current operation instance.  Phase 4 PR-7: takes
     * ScriptObject (was `kaguya::LuaRef`).  Lua-side callers still send
     * LuaRef and get wrapped at the guibridge.
     */
    void setOperation(lc::scripting::ScriptObject);

    void finishOperation();

    /**
     * \brief Register a Lua-side callback for @p event.  Phase 4 PR-9a:
     * wraps the LuaRef via makeLuaCallback and delegates to EventBus.
     * The `object-without-onEvent` guard is preserved verbatim
     * (luainterface.cpp:118 pre-refactor).
     */
    void registerEvent(const std::string& event, const kaguya::LuaRef& callback);

    /**
     * \brief Register a native/neutral ScriptCallback for @p event.
     * Phase 4 PR-9a — the native side into EventBus for phase 5 Python
     * scripts and for C++ code that already has a ScriptCallback.
     */
    void registerEvent(const std::string& event, lc::scripting::ScriptCallback callback);

    void deleteEvent(const std::string& event, const kaguya::LuaRef& callback);

    /**
     * \brief Trigger @p event with a LuaRef payload.  Phase 4 PR-9a:
     * converts the LuaRef payload to ScriptValue via fromLua and
     * delegates to EventBus, which materializes back to Lua per-callback.
     * Matches the pre-refactor behavior exactly.
     */
    void triggerEvent(const std::string& event, kaguya::LuaRef args);

    /**
     * \brief Trigger @p event with a native ScriptValue payload.
     * Phase 4 PR-9a — used by PR-9b's trigger* slots in MainWindow to
     * skip the Lua round-trip when payloads are already ScriptValues.
     */
    void triggerEvent(const std::string& event,
                      const lc::scripting::ScriptValue& args = lc::scripting::ScriptValue{});

    /**
     * \brief Number of listeners for @p event.  Phase 4 PR-9a — callers
     * building expensive payloads (mouseMove per pixel) query this and
     * skip payload materialization when zero.
     */
    std::size_t listenerCount(const std::string& event) const;

private:
    /**
     * \brief make common functions available globally and register finish events
     */
    void registerGlobalFunctions(QMainWindow* mainWindow);

private:
    kaguya::State _L;
    lc::lua::PluginManager _pluginManager;
    lc::scripting::ScriptObject _operation;
    // Phase 4 PR-9a — EventBus replaces the old
    // `std::map<std::string, std::vector<kaguya::LuaRef>> _events` and
    // its per-callback shape dispatch.  The ScriptCallback adapter
    // handles function-vs-table-with-onEvent per language.
    lc::scripting::EventBus _eventBus;
};
}
}
