#pragma once

#include <QDockWidget>
#include <imainwindow.h>

namespace Ui {
    class LuaScript;
}

class LuaScript : public QDockWidget {
        Q_OBJECT

    public:
        explicit LuaScript(IMainWindow* parent = 0);
        virtual ~LuaScript();

    private slots:
        void on_luaRun_clicked();

    private:
        Ui::LuaScript* ui;
        IMainWindow* _parent;
};

// LUASCRIPT_H
