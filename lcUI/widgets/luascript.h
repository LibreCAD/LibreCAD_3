#pragma once

#include <QDockWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include "cadmdichild.h"

namespace Ui {
    class LuaScript;
}

class LuaScript : public QDockWidget {
        Q_OBJECT

    public:
        explicit LuaScript(QMdiArea* mdiArea = 0);
        virtual ~LuaScript();

    private slots:
        void on_luaRun_clicked();

    private:
        Ui::LuaScript* ui;
        QMdiArea* _mdiArea;
};

// LUASCRIPT_H
