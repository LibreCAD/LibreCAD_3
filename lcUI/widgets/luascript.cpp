#include "luascript.h"
#include "ui_luascript.h"

#include <lclua.h>
#include <luainterface.h>

LuaScript::LuaScript(QMdiArea* mdiArea, CliCommand* cliCommand) :
    ui(new Ui::LuaScript),
    _mdiArea(mdiArea),
	_cliCommand(cliCommand) {
    ui->setupUi(this);
}

LuaScript::~LuaScript() {
    delete ui;
}


void LuaScript::on_luaRun_clicked() {
	if (QMdiSubWindow* activeSubWindow = _mdiArea->activeSubWindow()) {
        CadMdiChild* mdiChild = qobject_cast<CadMdiChild*>(activeSubWindow->widget());

		auto luaState = LuaIntf::LuaState::newState();
		auto lcLua = lc::LCLua(luaState);
		lcLua.setF_openFileDialog(&LuaInterface::openFileDialog);
		lcLua.addLuaLibs();
		lcLua.importLCKernel();
        lcLua.setDocument(mdiChild->document());

        auto out = lcLua.runString(ui->luaInput->toPlainText().toStdString().c_str());

		_cliCommand->write(QString::fromStdString(out));
	}
}

void LuaScript::on_open_clicked() {
	auto fileName = QFileDialog::getOpenFileName(
		0,
		tr("Open File"),
		QString(),
		tr("Lua (*.lua)")
	);

	if(!fileName.isEmpty()) {
		QFile file(fileName);

		file.open(QFile::ReadOnly | QFile::Text);
		QTextStream stream(&file);

		ui->luaInput->setPlainText(stream.readAll());

		file.close();
	}
}

void LuaScript::on_save_clicked() {
	auto fileName = QFileDialog::getSaveFileName(
		0,
		tr("Save File"),
		QString(),
		tr("Lua (*.lua)")
	);

	if(!fileName.isEmpty()) {
		QFile file(fileName);

		file.open(QFile::WriteOnly | QFile::Text);
		QTextStream stream(&file);

		stream << ui->luaInput->toPlainText();

		file.close();
	}
}