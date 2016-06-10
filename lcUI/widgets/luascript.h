#pragma once

#include <QDockWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QFileDialog>
#include <QTextStream>
#include "cadmdichild.h"
#include "clicommand.h"

namespace Ui {
    class LuaScript;
}

class LuaScript : public QWidget {
        Q_OBJECT

    public:
        explicit LuaScript(QMdiArea* mdiArea, CliCommand* cliCommand);
        virtual ~LuaScript();

    private slots:
        void on_luaRun_clicked();
		void on_open_clicked();
		void on_save_clicked();

    private:
        Ui::LuaScript* ui;
        QMdiArea* _mdiArea;
		CliCommand* _cliCommand;
};

// LUASCRIPT_H
