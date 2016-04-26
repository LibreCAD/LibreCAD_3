#include "luascript.h"
#include "ui_luascript.h"

#include <string>
#include <lcadluascript.h>

LuaScript::LuaScript(QMdiArea* mdiArea) :
    ui(new Ui::LuaScript),
    _mdiArea(mdiArea) {
    ui->setupUi(this);
}

LuaScript::~LuaScript() {
    delete ui;
}


void LuaScript::on_luaRun_clicked() {
	if (QMdiSubWindow* activeSubWindow = _mdiArea->activeSubWindow()) {
        CadMdiChild* mdiChild = qobject_cast<CadMdiChild*>(activeSubWindow->widget());

		LCadLuaScript lc(mdiChild->document());

		std::string out = lc.run(ui->luaInput->toPlainText().toStdString());
		ui->luaOutput->clear();
		ui->luaOutput->textCursor().insertText(QString::fromStdString(out));
	}
}

