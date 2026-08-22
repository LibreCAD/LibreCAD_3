#include "scriptdock.h"
#include "ui_scriptdock.h"

#include <lua/guibridge.h>

#include <QComboBox>
#include <QFileInfo>

#ifdef LC_WITH_PYTHONSCRIPT
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <lcpython.h>   // PythonInit
// Phase 5 PR-5.1 fixup: pyeventhooks.h include no longer needed here —
// installEventHooks() moved to MainWindow ctor.
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
    // Phase 3 PR-3.2 — lcgui also pre-imported, exposed as `gui` in
    // the namespace for Lua parity (Lua scripts also write `gui.*`).
    py::exec("import lc\nimport lcgui as gui\n", ns);
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
    // Phase 5 PR-5.1 fixup — the lc.event.register/deregister hook
    // install moved OUT of here and into MainWindow's ctor.  Rationale:
    // Python operations registering listeners at startup fire before
    // ScriptDock has ever been opened, so gating the install on
    // "user clicked Run script" silently no-op'd every early
    // registration.  MainWindow's install is unconditional AND
    // idempotent (setEventRegisterHook overwrites).
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
        // Phase 3 PR-3.2 — inject mainWindow + document freshly on
        // every Run click.  `mainWindow` is a raw pointer to the same
        // MainWindow instance Qt owns (lcgui's binding is HOLDER-LESS
        // so pybind11 doesn't try to delete it).  `document` is
        // refreshed from `_mdiChild->document()` mirroring the Lua
        // path at luascript.cpp:29-31 — a fresh newDocument()/openFile
        // between Run clicks changes the document instance, so we
        // re-inject to avoid holding a stale reference.
        _pyNamespace->ns["mainWindow"] =
            py::cast(_mainWindow, py::return_value_policy::reference);
        _pyNamespace->ns["document"] = _mdiChild->document();
        py::exec(code, _pyNamespace->ns);
    } catch (const py::error_already_set& e) {
        _cliCommand->write(std::string(e.what()));
    }
}
#endif

void ScriptDock::run() {
    const std::string code = ui->luaInput->toPlainText().toStdString();
#ifdef LC_WITH_PYTHONSCRIPT
    if (isPythonSelected()) {
        runPython(code);
        return;
    }
#endif
    runLua(code);
}

void ScriptDock::setInputText(const QString& text) {
    ui->luaInput->setPlainText(text);
}

void ScriptDock::on_luaRun_clicked() {
    // Phase 3 PR-3.4 — the slot is now a thin wrapper over the public
    // `run()` entry point so tests can drive the widget without
    // depending on Qt's signal delivery.
    run();
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
