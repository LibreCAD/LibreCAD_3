#include "luascript.h"
#include "ui_luascript.h"

#include <string>
#include <lcadluascript.h>

LuaScript::LuaScript(IMainWindow* parent) :
    QDockWidget(parent),
    ui(new Ui::LuaScript),
    _parent(parent) {
    ui->setupUi(this);
}

LuaScript::~LuaScript() {
    delete ui;
}


void LuaScript::on_luaRun_clicked() {
    LCadLuaScript lc(_parent->activeMdiChild()->document(), _parent->activeMdiChild()->storageManager());

    std::string out = lc.run(ui->luaInput->toPlainText().toStdString());
    ui->luaOutput->clear();
    ui->luaOutput->textCursor().insertText(QString::fromStdString(out));
}

