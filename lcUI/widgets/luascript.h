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

namespace Ui {
class LuaScript;
}

namespace lc {
namespace ui {
namespace widgets {
/**
 * \brief Widget that allows to enter and run Lua code.
 * This widget runs the code on the selected window in CadMdiChild and display the output in the command line.
 */
class LuaScript : public QWidget {
    Q_OBJECT

public:
    /**
     * \brief Create widget
     * \param mdiArea Pointer to QMdiArea
     * \param cliCommand Pointer to CliCommand
     */
    explicit LuaScript(lc::ui::MainWindow* mainWindow);

    virtual ~LuaScript();

private slots:

    /**
     * \brief Run script
     */
    void on_luaRun_clicked();

    /**
     * \brief Open file
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

private:
    Ui::LuaScript* ui;
    lc::ui::MainWindow* _mainWindow;
    CadMdiChild* _mdiChild;
    CliCommand* _cliCommand;
    kaguya::State luaState;
};
}
}
}
