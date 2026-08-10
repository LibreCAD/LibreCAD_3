#include "scriptdock.h"
#include "ui_scriptdock.h"

#include <lua/guibridge.h>

#include <QComboBox>
#include <QFileInfo>

#ifdef LC_WITH_PYTHONSCRIPT
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <lcpython.h>   // PythonInit
#endif

using namespace lc::ui::widgets;

namespace py = pybind11;

// Phase 3 PR-3.1 — the language combo entries are indexed by combo
// row order to keep the isPythonSelected check trivial.
namespace {
    constexpr int kLangLuaIndex    = 0;
    constexpr int kLangPythonIndex = 1;
}

#ifdef LC_WITH_PYTHONSCRIPT

ScriptDock::PyNamespace::PyNamespace() {
    // Per phase-1's permanent-release GIL pattern: the main thread does
    // NOT hold the GIL between Python calls.  Acquire here to build the
    // per-widget namespace.  PythonInit is idempotent — safe to call
    // once per ScriptDock even if lcpythonscript already booted the
    // interpreter for a headless CLI use.
    lc::python::PythonInit::initialize();
    py::gil_scoped_acquire gil;
    ns = py::dict();
    ns["__builtins__"] = py::module_::import("builtins");
    // Pre-import lc so `lc.entity.Line(...)` etc are directly available
    // — same as the Lua path where `lc.*` is exposed via LCLua.
    py::exec("import lc\n", ns);
    // `gui = lcgui` pre-import + mainWindow/document injection land in
    // phase 3 PR-3.2 when the lcgui embedded module exists.  For now
    // Python scripts can exercise the kernel (lc.entity, lc.operation,
    // lc.geo) but not the GUI-side widget API — matches the sub-plan's
    // "phase-3.2 fills lcgui" split.
}

ScriptDock::PyNamespace::~PyNamespace() {
    // GIL-holding destruction: the main thread's Qt event loop runs
    // GIL-free between Python calls, so ~py::dict must reacquire.
    py::gil_scoped_acquire gil;
    ns = py::dict();
}

#endif   // LC_WITH_PYTHONSCRIPT

ScriptDock::ScriptDock(lc::ui::MainWindow* mainWindow) :
    ui(new Ui::ScriptDock),
    _mainWindow(mainWindow),
    _mdiChild(mainWindow->cadMdiChild()),
    _cliCommand(mainWindow->cliCommand()) {
    ui->setupUi(this);

    auto lcLua = lc::lua::LCLua(luaState.state());
    lcLua.setF_openFileDialog(&LuaInterface::openFileDialog);
    lcLua.addLuaLibs();
    lcLua.importLCKernel();
    luaOpenGUIBridge(luaState.state());
    registerGlobalFunctions(luaState);

    // Language combo: Lua first (default), Python if available.  The
    // Python entry is hidden when built without LC_WITH_PYTHONSCRIPT
    // so the combo shows only Lua and isPythonSelected always returns
    // false — matches the sub-plan's "combo shows only Lua when
    // WITH_PYTHONSCRIPT=OFF" exit criterion.
    if (ui->language != nullptr) {
        ui->language->addItem(tr("Lua"));
#ifdef LC_WITH_PYTHONSCRIPT
        ui->language->addItem(tr("Python"));
#endif
    }

#ifdef LC_WITH_PYTHONSCRIPT
    _pyNamespace = std::make_unique<PyNamespace>();
#endif
}

ScriptDock::~ScriptDock() {
    // _pyNamespace's dtor acquires the GIL (see PyNamespace::~PyNamespace);
    // unique_ptr fires it before Qt's parent-child destruction kicks
    // in for `ui`.
    delete ui;
}

bool ScriptDock::isPythonSelected() const {
    return ui->language != nullptr
        && ui->language->currentIndex() == kLangPythonIndex;
}

void ScriptDock::runLua(const std::string& code) {
    auto lcLua = lc::lua::LCLua(luaState.state());
    lcLua.setDocument(_mdiChild->document());
    auto out = lcLua.runString(code.c_str());
    _cliCommand->write(out);
}

#ifdef LC_WITH_PYTHONSCRIPT
void ScriptDock::runPython(const std::string& code) {
    py::gil_scoped_acquire gil;
    try {
        // Phase 3 PR-3.1 — mainWindow / document injection defers to
        // 3.2 (lcgui pybind11 bindings for MainWindow, CadMdiChild,
        // Document).  For 3.1 the Python leg runs against `lc.*` only
        // — enough to smoke-test the interpreter path and combo
        // switching.
        py::exec(code, _pyNamespace->ns);
    } catch (const py::error_already_set& e) {
        _cliCommand->write(std::string(e.what()));
    }
}
#endif

void ScriptDock::on_luaRun_clicked() {
    const std::string code = ui->luaInput->toPlainText().toStdString();
#ifdef LC_WITH_PYTHONSCRIPT
    if (isPythonSelected()) {
        runPython(code);
        return;
    }
#endif
    runLua(code);
}

void ScriptDock::on_open_clicked() {
    auto fileName = QFileDialog::getOpenFileName(
                        nullptr,
                        tr("Open File"),
                        QString(),
                        tr("Scripts (*.lua *.py);;Lua (*.lua);;Python (*.py)")
                    );

    if(!fileName.isEmpty()) {
        QFile file(fileName);

        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);

        ui->luaInput->setPlainText(stream.readAll());

        file.close();

        // Auto-switch combo on file extension so subsequent Run
        // picks the right engine without an extra click.
#ifdef LC_WITH_PYTHONSCRIPT
        if (ui->language != nullptr) {
            const QString ext = QFileInfo(fileName).suffix().toLower();
            if (ext == "py") {
                ui->language->setCurrentIndex(kLangPythonIndex);
            } else if (ext == "lua") {
                ui->language->setCurrentIndex(kLangLuaIndex);
            }
        }
#endif
    }
}

void ScriptDock::on_save_clicked() {
    auto fileName = QFileDialog::getSaveFileName(
                        nullptr,
                        tr("Save File"),
                        QString(),
                        tr("Scripts (*.lua *.py);;Lua (*.lua);;Python (*.py)")
                    );

    if(!fileName.isEmpty()) {
        QFile file(fileName);

        file.open(QFile::WriteOnly | QFile::Text);
        QTextStream stream(&file);

        stream << ui->luaInput->toPlainText();

        file.close();
    }
}

void ScriptDock::registerGlobalFunctions(kaguya::State& luaState) {
    // register common functions i.e. run_basic_operation and message
    luaState["mainWindow"] = static_cast<lc::ui::MainWindow*>(_mainWindow);
    luaState.dostring("run_basic_operation = function(operation, init_method) mainWindow:runOperation(operation, init_method) end");

    // cli command helper functions
    luaState.dostring("message = function(m) mainWindow:cliCommand():write(m) end");
    luaState.dostring("add_command = function(command, callback) mainWindow:cliCommand():addCommand(command, callback) end");
    luaState.dostring("run_command = function(command) mainWindow:cliCommand():runCommand(command) end");
    luaState.dostring("CreateDialogWidget = function(widgetName) return gui.DialogWidget(widgetName,mainWindow) end");
}
