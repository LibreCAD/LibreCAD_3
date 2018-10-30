#include "luascript.h"
#include "ui_luascript.h"

#include <lclua.h>
#include <luainterface.h>

using namespace lc::ui::widgets;

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
        auto mdiChild = qobject_cast<lc::ui::CadMdiChild*>(activeSubWindow->widget());

		kaguya::State luaState;
        auto lcLua = lc::lua::LCLua(luaState.state());
        lcLua.setF_openFileDialog(&LuaInterface::openFileDialog);
        lcLua.addLuaLibs();
        lcLua.importLCKernel();
        lcLua.setDocument(mdiChild->document());
    }
}

void LuaScript::on_open_clicked() {
	auto fileName = QFileDialog::getOpenFileName(
		nullptr,
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
		nullptr,
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