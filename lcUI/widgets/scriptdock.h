#pragma once

#include <QDockWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QFileDialog>
#include <QTextStream>
#include <mainwindow.h>
#include "cadmdichild.h"
#include "clicommand.h"

#include <lclua.h>

#include <memory>

namespace Ui {
class ScriptDock;
}

namespace lc {
namespace ui {
namespace widgets {
/**
 * \brief Widget that allows to enter and run script code in either Lua
 * or Python.  Phase 3 PR-3.1 — was `LuaScript`; renamed + generalized
 * with a language QComboBox.  The Lua-visible constructor name
 * `lc.LuaScript` remains bound as an alias for backward compat with
 * existing Lua scripts (see guibridge.cpp).  Python entry appears
 * only when built with LC_WITH_PYTHONSCRIPT.
 *
 * Runs code on the selected window in CadMdiChild and displays output
 * in the command line.
 */
class ScriptDock : public QWidget {
    Q_OBJECT

public:
    /**
     * \brief Create widget
     * \param mainWindow Pointer to MainWindow
     */
    explicit ScriptDock(lc::ui::MainWindow* mainWindow);

    virtual ~ScriptDock();

    /**
     * \brief Run the current text of the input pane in the selected
     * language.  Public entry point that dispatches to `runLua` /
     * `runPython`.  Tests use this directly (avoids depending on the
     * Qt event loop to deliver a button click); the "Run" button's
     * slot is a thin wrapper.
     */
    void run();

    /**
     * \brief Set the input text programmatically.  Used by the dual-
     * language ScriptDock test (PR-3.4).
     */
    void setInputText(const QString& text);

private slots:

    /**
     * \brief Run script (dispatches to Lua or Python by combo state)
     */
    void on_luaRun_clicked();

    /**
     * \brief Open file (auto-switches language on extension)
     */
    void on_open_clicked();

    /**
     * \brief Save file
     */
    void on_save_clicked();

private:
    /**
     * \brief Register helper global functions
     */
    void registerGlobalFunctions(kaguya::State& luaState);

    /**
     * \brief Run current text as Lua.
     */
    void runLua(const std::string& code);

#ifdef LC_WITH_PYTHONSCRIPT
    /**
     * \brief Run current text as Python.  Namespace persists across
     * clicks (mirrors the Lua state's persistence).
     */
    void runPython(const std::string& code);
#endif

    /**
     * \brief True iff the language combo has selected Python.
     */
    bool isPythonSelected() const;

private:
    Ui::ScriptDock* ui;
    lc::ui::MainWindow* _mainWindow;
    CadMdiChild* _mdiChild;
    CliCommand* _cliCommand;
    kaguya::State luaState;

#ifdef LC_WITH_PYTHONSCRIPT
    // Defined in scriptdock.cpp to keep pybind11 types out of this Qt/MOC
    // header.  Its destructor acquires the GIL before releasing its dict.
    struct PyNamespace;
    std::unique_ptr<PyNamespace> _pyNamespace;
#endif
};
}
}
}
