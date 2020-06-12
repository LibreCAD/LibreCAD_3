#include "luascript.h"
#include "ui_luascript.h"

using namespace lc::ui::widgets;

LuaScript::LuaScript(lc::ui::MainWindow* mainWindow) :
    ui(new Ui::LuaScript),
    _mainWindow(mainWindow),
	_mdiChild(mainWindow->cadMdiChild()),
	_cliCommand(mainWindow->cliCommand()),
    _lcLua(new lc::lua::LCLua(mainWindow->luaInterface()->luaState())){
    ui->setupUi(this);
}

LuaScript::~LuaScript() {
    delete ui;
}


void LuaScript::on_luaRun_clicked() {
    _lcLua->setDocument(_mdiChild->document());

    auto out = _lcLua->runString(ui->luaInput->toPlainText().toStdString().c_str());
    _cliCommand->write(out);
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

void LuaScript::registerGlobalFunctions(kaguya::State& luaState) {
    // register common functions i.e. run_basic_operation and message
    luaState.dostring("run_basic_operation = function(operation, init_method) mainWindow:runOperation(operation, init_method) end");

    // cli command helper functions
    luaState.dostring("message = function(m) mainWindow:cliCommand():write(m) end");
    luaState.dostring("add_command = function(command, callback) mainWindow:cliCommand():addCommand(command, callback) end");
    luaState.dostring("run_command = function(command) mainWindow:cliCommand():runCommand(command) end");
}
