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

    void registerEvent(const std::string& event, const kaguya::LuaRef& callback);

    void deleteEvent(const std::string& event, const kaguya::LuaRef& callback);

    void triggerEvent(const std::string& event, kaguya::LuaRef args);

private:
    /**
     * \brief make common functions available globally and register finish events
     */
    void registerGlobalFunctions(QMainWindow* mainWindow);

private:
    kaguya::State _L;
    lc::lua::PluginManager _pluginManager;
    lc::scripting::ScriptObject _operation;
    std::map<std::string, std::vector<kaguya::LuaRef>> _events;
};
}
}
