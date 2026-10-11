#include "operationloader.h"

#include "mainwindow.h"
#include "widgets/clicommand.h"
#include "widgets/toolbar.h"
#include "managers/contextmenumanager.h"

// Phase 4 PR-7 — 8 dostring codegen sites killed; native lambdas call
// MainWindow::runOperationByName + currentOperation.
#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptobject.h>

// Phase 5 PR-5.2 — Python second source: iterates lc.operation_registry
// and wires each entry into the same CliCommand/Toolbar/Menu paths the
// Lua loader uses.  Uses the Python adapter to wrap operation classes
// as ScriptObject for the ordered resolver list.
#ifdef LC_WITH_PYTHONSCRIPT
#include <qt_keywords_push.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <qt_keywords_pop.h>
#include <scriptadapter/pythonobject.h>
#include <algorithm>
#include <vector>
#endif

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QString>
#include <QStringList>

using namespace lc::ui;

namespace
{
const char* operationsLua = "/actions/operations.lua";

class FolderFinder
{
public:
    std::string operator () (std::string inputFolder) const
    {
        std::string ret = searchSubFolders(inputFolder);
        if (!ret.empty())
            return ret;

        std::string binFolder = QDir{QCoreApplication::applicationDirPath()}.canonicalPath().toStdString();
        std::cout<<"binFolder: "<<binFolder<<std::endl;
        ret = searchSubFolders(binFolder);
        if (!ret.empty())
            return ret;
        return {};
    }
private:
    std::string searchSubFolders(std::string folder) const
    {
        for (const char* subFolder: {"/lcUILua", "/../share/librecad/lcUILua",
                                     "/../Resources/librecad/lcUILua"})
            if (isValid(folder + subFolder))
                return folder + subFolder;

        QDirIterator it(QString{folder.c_str()}, QDirIterator::Subdirectories);

          // Iterate through the directory using the QDirIterator
          while (it.hasNext())
          {
              QString child = it.next();
              QFileInfo file(child);
              if (!file.isDir())
                  continue;

              if (isValid(child.toStdString()))
                  return child.toStdString();
          }
          return {};
    }

    bool isValid(const std::string& path) const
    {
        std::cout<<"Test "<<path + operationsLua<<": "<<QFileInfo{QString{path.c_str()} + operationsLua}.exists()<<std::endl;
        return QFileInfo{QString{path.c_str()} + operationsLua}.exists();
    }
};
}


OperationLoader::OperationLoader(const std::string& luaPath, QMainWindow* qmainWindow, kaguya::State& luaState)
    :
    _L(luaState),
    qmainWindow(qmainWindow)
{
    std::string path = FolderFinder{}(luaPath);
    std::cout<<"updated path:: "<<path<<std::endl;
    loadLuaOperations(path);
}

void OperationLoader::loadLuaOperations(const std::string& luaPath) {
    // call operations and createoperations first
    _L.dofile(luaPath + "/actions/operations.lua");
    _L.dofile(luaPath + "/createActions/createOperations.lua");
    loadLuaFolder("createActions", "createOperations", luaPath);

    // create list of all operations in creationGroup and dimensionsGroup
    getSetOfGroupElements();
    loadLuaFolder("actions", "operations", luaPath);

    // fetch a list of all keys from the state table
    kaguya::LuaTable globalTable(_L.globalTable());
    std::vector<kaguya::LuaRef> globalKeys = globalTable.keys();

    // list of properties to look out for in the operation
    std::set<std::string> initProperties = {
        "init",
        "command_line",
        "menu_actions",
        "icon",
        "description",
        "operation_options",
        "context_transitions"
    };

    /* Loop through all keys to search for the ones containing "Operation" in their name,
     * Call initialize operation with list of all found properties for the operation
     */
    for (kaguya::LuaRef v : globalKeys)
    {
        if (v.isType<std::string>())
        {
            std::string vkey = std::string(v.get<std::string>());

            // if Operation is found in key e.g. LineOperations
            if (vkey.find("Operation") < vkey.length() && _L[vkey].type() == _L[vkey].TYPE_TABLE) {
                kaguya::LuaTable opTable = _L[vkey];
                foundProperties[vkey] = std::set<std::string>();

                for (kaguya::LuaRef& op : opTable.keys())
                {
                    if (op.isType<std::string>())
                    {
                        std::string opkey = std::string(op.get<std::string>());

                        // if property is in properties we are looking for, add it in foundProperties
                        if (initProperties.find(opkey) != initProperties.end()) {
                            foundProperties[vkey].insert(opkey);
                        }
                    }
                }

                // determine group name for operation
                if (groupElements.at("creationGroupElements").find(vkey) != groupElements.at("creationGroupElements").end()) {
                    groupNames[vkey] = "Creation";
                }
                else if (groupElements.at("dimensionsGroupElements").find(vkey) != groupElements.at("dimensionsGroupElements").end()) {
                    groupNames[vkey] = "Dimensions";
                }
                else {
                    // if not in creation group or dimension group, it has to be in the modify group
                    groupNames[vkey] = "Modify";
                }

                initializeOperation(vkey);
            }
        }
    }

    _L["run_op"] = nullptr;
}

void OperationLoader::loadLuaFolder(const std::string folderName, const std::string& fileToSkip, const std::string& luaPath) {
    QDir folderDir((luaPath + "/" + folderName).c_str());
    QStringList luaFiles = folderDir.entryList(QStringList() << "*.lua", QDir::Files);
    for (QString str : luaFiles)
    {
        std::string filename = str.toStdString();
        // skip fileToSkip.lua as it has been already called
        if (str.toStdString() != fileToSkip) {
            _L.dofile(luaPath + "/" + folderName + "/" + filename);
        }
    }
}

void OperationLoader::getSetOfGroupElements() {
    groupElements["creationGroupElements"] = std::set<std::string>();
    groupElements["dimensionsGroupElements"] = std::set<std::string>();

    // insert elements into their respective sets
    for (kaguya::LuaRef v : _L.globalTable().keys())
    {
        if (v.isType<std::string>()) {
            std::string vkey = std::string(v.get<std::string>());

            if (vkey.find("Operation") < vkey.length()) {
                if (vkey.find("Dim") < vkey.length()) {
                    groupElements["dimensionsGroupElements"].insert(vkey);
                }
                else {
                    groupElements["creationGroupElements"].insert(vkey);
                }
            }
        }
    }
}

void OperationLoader::initializeOperation(const std::string& vkey)
{
    for (const std::string& opkey : foundProperties[vkey])
    {
        // init function
        if (opkey == "init") {
            // eg. _L["LineOperations"]["init"]()
            _L[vkey][opkey]();
        }

        // command line
        if (opkey == "command_line") {
            addOperationCommandLine(vkey, opkey);
        }

        // menu actions
        if (opkey == "menu_actions") {
            addOperationMenuAction(vkey, opkey);
        }

        // Toolbar attributes
        if (opkey == "icon") {
            addOperationIcon(vkey, opkey);
        }

        // operation icons
        if (opkey == "operation_options") {
            addOperationToolbarOptions(vkey, opkey);
        }

        // context transitions
        if (opkey == "context_transitions") {
            addContextTransitions(vkey, opkey);
        }
    }

    addContextMenuOperations(vkey);
}


void OperationLoader::addOperationCommandLine(const std::string& vkey, const std::string& opkey) {
    // Phase 4 PR-7 — 3 `run_op = function() run_basic_operation(...) end`
    // dostrings killed; native lambdas capture `vkey` and (optionally) a
    // computed init-method string.
    widgets::CliCommand* cliCommand = static_cast<MainWindow*>(qmainWindow)->cliCommand();
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);

    if (_L[vkey][opkey].type() == _L[vkey][opkey].TYPE_STRING) {
        cliCommand->addCommand(_L[vkey][opkey],
            lc::scripting::nativeCallback([mWindow, vkey]() {
                mWindow->runOperationByName(vkey);
            }));
    }

    if (_L[vkey][opkey].type() == _L[vkey][opkey].TYPE_TABLE) {
        std::vector<kaguya::LuaRef> commandList = _L[vkey][opkey].keys();

        for (kaguya::LuaRef commandKey : commandList) {
            std::string key = commandKey.get<std::string>();

            // if key is digits only i.e. if no key provided, connect it to default init
            if (std::find_if(key.begin(), key.end(), [](unsigned char c) {
            return !std::isdigit(c);
            }) == key.end())
            {
                // connect to default init function
                cliCommand->addCommand(_L[vkey][opkey][commandKey].get<std::string>().c_str(),
                    lc::scripting::nativeCallback([mWindow, vkey]() {
                        mWindow->runOperationByName(vkey);
                    }));
            }
            else {
                // connect to provided init function.  Suffix comes from
                // the Lua-side operation-class table entry — precompute
                // "_init_<suffix>" so the lambda holds a plain string.
                std::string initMethod = "_init_" + _L[vkey][opkey][commandKey].get<std::string>();
                cliCommand->addCommand(key.c_str(),
                    lc::scripting::nativeCallback([mWindow, vkey, initMethod]() {
                        mWindow->runOperationByName(vkey, initMethod);
                    }));
            }
        }
    }
}

void OperationLoader::addOperationMenuAction(const std::string& vkey, const std::string& opkey) {
    // Phase 4 PR-7 — 2 `run_op` dostrings killed.
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);
    std::map<std::string, std::string> map = _L[vkey][opkey];

    for (auto element : map)
    {
        if (element.first == "default") {
            mWindow->connectMenuItem(element.second,
                lc::scripting::nativeCallback([mWindow, vkey]() {
                    mWindow->runOperationByName(vkey);
                }));
        }
        else {
            std::string initMethod = "_init_" + element.first;
            mWindow->connectMenuItem(element.second,
                lc::scripting::nativeCallback([mWindow, vkey, initMethod]() {
                    mWindow->runOperationByName(vkey, initMethod);
                }));
        }
    }
}

void OperationLoader::addOperationIcon(const std::string& vkey, const std::string& opkey) {
    // Phase 4 PR-7 — 1 `run_op` dostring killed.
    widgets::Toolbar* toolbar = static_cast<MainWindow*>(qmainWindow)->toolbar();
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);
    std::string icon = _L[vkey][opkey].get<std::string>();
    std::string tooltip;

    // if description not provided, use operation name
    if (foundProperties[vkey].find("description") != foundProperties[vkey].end()) {
        tooltip = _L[vkey]["description"].get<std::string>();
    }
    else {
        tooltip = vkey.substr(0, vkey.find("Operation"));
    }

    std::string iconPath = ":/icons/" + icon;

    toolbar->addButton(vkey.c_str(), iconPath.c_str(), groupNames[vkey].c_str(),
        lc::scripting::nativeCallback([mWindow, vkey]() {
            mWindow->runOperationByName(vkey);
        }),
        tooltip.c_str());
}

void OperationLoader::addOperationToolbarOptions(const std::string& vkey, const std::string& opkey) {
    // Phase 4 PR-7 — 2 `operation_op` dostrings killed.  Each toolbar-
    // option button becomes a native ScriptCallback capturing:
    //   * the icon path (per-button),
    //   * the option label (per-button),
    //   * the method name to invoke on the CURRENT operation instance
    //     (looked up at click time via MainWindow::currentOperation()).
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);
    std::map<std::string, kaguya::LuaRef> options = _L[vkey][opkey];

    // Helper: build a ScriptCallback that, when fired, spawns a toolbar
    // Cancel-group button whose click callback in turn invokes the named
    // method on the current operation.  This is the neutral analog of
    // the legacy `mainWindow:toolbar():addButton(..., function()
    // luaInterface:operation():<action>() end, ...)`  double-nested
    // closure.
    auto makeOptionCallback = [mWindow](std::string iconPath,
                                        std::string label,
                                        std::string action) {
        return lc::scripting::nativeCallback([mWindow, iconPath, label, action]() {
            mWindow->toolbar()->addButton(
                "cancel", iconPath.c_str(), "Current operation",
                lc::scripting::nativeCallback([mWindow, action]() {
                    lc::scripting::ScriptObject op = mWindow->currentOperation();
                    if (!op.isNil()) op.callMethod(action);
                }),
                label.c_str());
        });
    };

    std::vector<lc::scripting::ScriptCallback> optionsList;
    for (auto element : options) {
        // operation_options for init_method
        if (element.first.find("_init") < element.first.size()) {
            std::map<std::string, kaguya::LuaRef> optionsInit = element.second;

            std::vector<lc::scripting::ScriptCallback> optionsInitList;
            for (auto elementInit : optionsInit) {
                std::map<std::string, std::string> optionInit = elementInit.second;
                std::string iconPath = ":/icons/" + optionInit["icon"];
                optionsInitList.push_back(
                    makeOptionCallback(iconPath, elementInit.first, optionInit["action"]));
            }

            // LINEOPERATIONS_init_pal - example key for operation options list
            mWindow->addOperationOptions(
                _L[vkey]["command_line"].get<std::string>() + element.first,
                std::move(optionsInitList));
        }
        else
        {
            // default operation_options
            std::map<std::string, std::string> option = element.second;
            std::string iconPath = ":/icons/" + option["icon"];
            optionsList.push_back(
                makeOptionCallback(iconPath, element.first, option["action"]));
        }
    }

    // provide options list to mainWindow so it can run necessary function on runOperation
    if (optionsList.size() > 0) {
        mWindow->addOperationOptions(
            _L[vkey]["command_line"].get<std::string>(),
            std::move(optionsList));
    }
}

void OperationLoader::addContextMenuOperations(const std::string& vkey) {
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);
    lc::ui::ContextMenuManager* contextMenuManager = ContextMenuManager::GetContextMenuManager(mWindow->contextMenuManagerId());
    contextMenuManager->addOperation(vkey, groupNames[vkey]);
}

void OperationLoader::addContextTransitions(const std::string& vkey, const std::string& opkey) {
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);
    lc::ui::ContextMenuManager* contextMenuManager = ContextMenuManager::GetContextMenuManager(mWindow->contextMenuManagerId());
    std::map<std::string, kaguya::LuaRef> functionNames = _L[vkey][opkey];

    for (auto fName : functionNames) {
        std::vector<kaguya::LuaRef> transitionList = _L[vkey][opkey][fName.first];
        std::vector<std::string> transList;

        for (kaguya::LuaRef transitionFunc : transitionList) {
            transList.push_back(transitionFunc.get<std::string>());
        }

        contextMenuManager->addTransition(_L[vkey]["name"], fName.first, transList);
    }
}

#ifdef LC_WITH_PYTHONSCRIPT

namespace {

namespace py = pybind11;

// Helper: read a string attribute (Python-side).  Returns empty
// string if attribute is missing or non-string.  Used for icon /
// description / etc. which the Lua path reads from the class table
// with a similar "missing = skip" pattern.
std::string pyAttrString(const py::object& cls, const char* name) {
    if (!py::hasattr(cls, name)) return {};
    try {
        py::object attr = cls.attr(name);
        if (attr.is_none()) return {};
        return py::str(attr);
    } catch (const py::error_already_set&) {
        return {};
    }
}

// Helper: determine the group ("Creation" | "Dimensions" | "Modify")
// for a Python operation.  Explicit `group = "..."` attribute wins;
// falls back to the same name-based rule Lua uses ("Dim" in the name
// → Dimensions), else "Creation" as the default.  Matches the
// sub-plan's "defaulting by the same name rules as Lua for
// familiarity" clause.
std::string pyOperationGroup(const py::object& cls, const std::string& name) {
    std::string explicitGroup = pyAttrString(cls, "group");
    if (!explicitGroup.empty()) {
        return explicitGroup;
    }
    if (name.find("Dim") != std::string::npos) {
        return "Dimensions";
    }
    return "Creation";
}

std::string pyOperationModuleDirectory(const py::object& cls) {
    try {
        if (!py::hasattr(cls, "__module__")) return {};
        const std::string moduleName = py::str(cls.attr("__module__"));
        py::object module = py::module_::import(moduleName.c_str());
        if (!py::hasattr(module, "__file__")) return {};
        const std::string moduleFile = py::str(module.attr("__file__"));
        const std::size_t separator = moduleFile.find_last_of("/\\");
        return separator == std::string::npos
            ? std::string{}
            : moduleFile.substr(0, separator);
    } catch (const py::error_already_set&) {
        // Resource-backed icons remain usable if a module has no filesystem
        // origin (for example, a built-in or test operation).
        return {};
    }
}

} // namespace

void OperationLoader::loadPythonOperations() {
    // Phase 5 PR-5.2 — the Python second source.  Walk
    // lc.operation_registry (populated by @lc.register_operation),
    // sort by name (matches Lua's std::set alphabetical wiring
    // order), and wire each entry into the same C++ paths the Lua
    // loader uses.  Push a Python-registry resolver onto MainWindow's
    // ordered resolver list so runOperationByName reaches Python ops.
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);
    if (mWindow == nullptr) return;

    py::gil_scoped_acquire gil;

    // 1) Register the Python-registry resolver.  It's pushed AFTER the
    //    Lua-globals resolver (registered in MainWindow ctor), so the
    //    reverse-walking dispatch in runOperationByName finds Python
    //    first — matches the phase-4 PR-7 "later wins" design.
    mWindow->registerOperationResolver(
        [](const std::string& name) -> lc::scripting::ScriptObject {
            py::gil_scoped_acquire innerGil;
            try {
                py::dict reg = py::reinterpret_borrow<py::dict>(
                    py::module_::import("lc").attr("operation_registry"));
                py::str pyName = py::cast(name);
                if (!reg.contains(pyName)) {
                    return lc::scripting::ScriptObject{};
                }
                py::object cls = reg[pyName];
                return lc::python::makePythonObject(cls);
            } catch (const py::error_already_set&) {
                return lc::scripting::ScriptObject{};
            }
        });

    // 2) Iterate the registry sorted by name.
    py::dict reg;
    try {
        reg = py::reinterpret_borrow<py::dict>(
            py::module_::import("lc").attr("operation_registry"));
    } catch (const py::error_already_set& e) {
        std::cerr << "[OperationLoader] failed to import lc.operation_registry: "
                  << e.what() << std::endl;
        return;
    }

    std::vector<std::string> names;
    for (auto item : reg) {
        try {
            names.emplace_back(py::str(item.first));
        } catch (const py::error_already_set&) {
            // Non-string key — skip; can't happen from the decorator,
            // but tolerate user manipulation of the dict.
        }
    }
    std::sort(names.begin(), names.end());

    // 3) Wire each operation.
    lc::ui::ContextMenuManager* contextMenuManager =
        ContextMenuManager::GetContextMenuManager(
            mWindow->contextMenuManagerId());
    widgets::CliCommand* cliCommand = mWindow->cliCommand();
    widgets::Toolbar* toolbar = mWindow->toolbar();

    for (const std::string& name : names) {
        // Name collision policy: registry key shares namespace with
        // Lua vkeys (toolbar labels, ui_settings, ContextMenuManager
        // _operationMap).  Reject duplicate with a logged warning.
        // `foundProperties` is populated by loadLuaOperations so its
        // key set IS the Lua vkey set.
        if (foundProperties.find(name) != foundProperties.end()) {
            std::cerr << "[OperationLoader] Python operation '" << name
                      << "' collides with a Lua operation of the same "
                         "name — Python registration skipped."
                      << std::endl;
            continue;
        }

        py::object cls;
        try {
            cls = reg[py::cast(name)];
        } catch (const py::error_already_set&) {
            continue;
        }

        const std::string group = pyOperationGroup(cls, name);
        const std::string operationModuleDirectory =
            pyOperationModuleDirectory(cls);
        // Record the group so ContextMenuManager iteration finds it
        // (mirrors the Lua path's groupNames map).
        groupNames[name] = group;

        // Optional `init` callable — run at load time (matches Lua).
        if (py::hasattr(cls, "init")) {
            try {
                py::object initFn = cls.attr("init");
                if (!initFn.is_none()) {
                    initFn();
                }
            } catch (const py::error_already_set& e) {
                std::cerr << "[OperationLoader] Python operation '"
                          << name << "' init() raised: " << e.what()
                          << std::endl;
            }
        }

        // ---- CliCommand (command_line) ----
        //
        // Phase 5 PR-5.5 fixup round 2 — CliCommand::addCommand returns
        // `bool` indicating whether the name was accepted; a duplicate
        // silently loses today (the CLI command map stores the FIRST
        // registration).  Log the collision here for parity with the
        // `foundProperties` name-collision path above — otherwise a
        // Python plugin registering the same CLI text as a Lua op
        // (e.g. "LINE") would silently no-op with zero diagnostic.
        auto addOrWarn = [&](const char* cmdName,
                             lc::scripting::ScriptCallback cb) {
            if (!cliCommand->addCommand(cmdName, std::move(cb))) {
                std::cerr << "[OperationLoader] Python operation '" << name
                          << "' CLI command '" << cmdName
                          << "' collides with an existing command — "
                             "Python registration silently rejected."
                          << std::endl;
            }
        };
        if (py::hasattr(cls, "command_line")) {
            py::object cmd = cls.attr("command_line");
            if (py::isinstance<py::str>(cmd)) {
                const std::string cmdStr = py::str(cmd);
                // PR-5.2 fixup — cmdStr is std::string but
                // CliCommand::addCommand takes `const char*`; needed
                // .c_str() (missing in the original PR-5.2 code was a
                // real compile error).
                addOrWarn(cmdStr.c_str(),
                    lc::scripting::nativeCallback([mWindow, name]() {
                        mWindow->runOperationByName(name);
                    }));
            } else if (py::isinstance<py::dict>(cmd)) {
                // PR-5.2 fixup — the ORIGINAL PR-5.2 code got this
                // mapping BACKWARDS.  Lua's convention (verified
                // against lcUILua/createActions/ellipseoperations.lua):
                //   `command_line = { "ELLIPSE", ARCELLIPSE = "arc" }`
                //   → iterated as:
                //     digit-only key 1  → value "ELLIPSE" is CLI text,
                //                         default init.
                //     non-digit key "ARCELLIPSE" → the KEY is the CLI
                //                                  text, VALUE is the
                //                                  init-suffix.
                // So typing "ARCELLIPSE" runs `_init_arc`.  Python's
                // dict has the same key/value shape as Lua's table;
                // preserve the mapping verbatim so a mixed-language
                // author's muscle memory carries over.
                for (auto item : py::reinterpret_borrow<py::dict>(cmd)) {
                    std::string key = py::str(item.first);
                    const std::string value = py::str(item.second);
                    // Digit-only key = default init; CLI text = value.
                    bool digitsOnly = !key.empty()
                        && std::all_of(key.begin(), key.end(),
                            [](unsigned char c) { return std::isdigit(c); });
                    if (digitsOnly) {
                        addOrWarn(value.c_str(),
                            lc::scripting::nativeCallback([mWindow, name]() {
                                mWindow->runOperationByName(name);
                            }));
                    } else {
                        // Non-digit key: CLI text = KEY, init suffix
                        // = VALUE.  Matches Lua at
                        // operationloader.cpp:280-281.
                        std::string initMethod = "_init_" + value;
                        addOrWarn(key.c_str(),
                            lc::scripting::nativeCallback(
                                [mWindow, name, initMethod]() {
                                    mWindow->runOperationByName(name, initMethod);
                                }));
                    }
                }
            }
        }

        // ---- Menu actions ----
        if (py::hasattr(cls, "menu_actions")) {
            py::object menuActions = cls.attr("menu_actions");
            if (py::isinstance<py::dict>(menuActions)) {
                for (auto item : py::reinterpret_borrow<py::dict>(menuActions)) {
                    std::string key = py::str(item.first);
                    std::string objectName = py::str(item.second);
                    if (key == "default") {
                        mWindow->connectMenuItem(objectName,
                            lc::scripting::nativeCallback([mWindow, name]() {
                                mWindow->runOperationByName(name);
                            }));
                    } else {
                        std::string initMethod = "_init_" + key;
                        mWindow->connectMenuItem(objectName,
                            lc::scripting::nativeCallback(
                                [mWindow, name, initMethod]() {
                                    mWindow->runOperationByName(name, initMethod);
                                }));
                    }
                }
            }
        }

        // ---- Toolbar icon ----
        if (py::hasattr(cls, "icon")) {
            const std::string icon = pyAttrString(cls, "icon");
            if (!icon.empty()) {
                const std::string tooltip =
                    !pyAttrString(cls, "description").empty()
                        ? pyAttrString(cls, "description")
                        : name.substr(0, name.find("Operation"));
                const std::string iconPath = ":/icons/" + icon;

                toolbar->addButton(name.c_str(), iconPath.c_str(),
                                   group.c_str(),
                                   lc::scripting::nativeCallback(
                                       [mWindow, name]() {
                                           mWindow->runOperationByName(name);
                                       }),
                                   tooltip.c_str(),
                                   /*checkable=*/false,
                                   /*tabName=*/"Quick Access",
                                   operationModuleDirectory.empty()
                                       ? nullptr
                                       : operationModuleDirectory.c_str());
            }
        }

        // ---- Operation-option toolbar buttons ----
        // Match the Lua schema: operation_options maps option labels to
        // {icon, action}. A key containing `_init` scopes those options to
        // that init method; MainWindow checks command_line + init_method
        // before falling back to the command-wide options.
        if (py::hasattr(cls, "operation_options")) {
            try {
                py::object rawOptions = cls.attr("operation_options");
                if (!py::isinstance<py::dict>(rawOptions)) {
                    std::cerr << "[OperationLoader] Python operation '"
                              << name
                              << "' operation_options must be a dict."
                              << std::endl;
                } else {
                    const py::dict scopes =
                        py::reinterpret_borrow<py::dict>(rawOptions);
                    const std::string commandLine =
                        pyAttrString(cls, "command_line");
                    std::vector<std::string> scopeNames;
                    for (const auto item : scopes) {
                        if (!py::isinstance<py::str>(item.first)) {
                            std::cerr << "[OperationLoader] Python operation '"
                                      << name
                                      << "' has a non-string operation_options key; skipping it."
                                      << std::endl;
                            continue;
                        }
                        scopeNames.emplace_back(py::cast<std::string>(item.first));
                    }
                    std::sort(scopeNames.begin(), scopeNames.end());

                    auto appendOptionCallback = [&] (
                            std::vector<lc::scripting::ScriptCallback>& callbacks,
                            const std::string& optionName, py::handle rawOption) {
                        if (!py::isinstance<py::dict>(rawOption)) {
                            std::cerr << "[OperationLoader] Python operation '"
                                      << name << "' option '" << optionName
                                      << "' must be a dict with icon/action."
                                      << std::endl;
                            return;
                        }
                        const py::dict fields =
                            py::reinterpret_borrow<py::dict>(rawOption);
                        const py::str iconKey("icon");
                        const py::str actionKey("action");
                        if (!fields.contains(iconKey) ||
                            !fields.contains(actionKey) ||
                            !py::isinstance<py::str>(fields[iconKey]) ||
                            !py::isinstance<py::str>(fields[actionKey])) {
                            std::cerr << "[OperationLoader] Python operation '"
                                      << name << "' option '" << optionName
                                      << "' requires string icon and action fields."
                                      << std::endl;
                            return;
                        }

                        const std::string icon =
                            py::cast<std::string>(fields[iconKey]);
                        const std::string action =
                            py::cast<std::string>(fields[actionKey]);
                        if (icon.empty() || action.empty()) {
                            std::cerr << "[OperationLoader] Python operation '"
                                      << name << "' option '" << optionName
                                      << "' requires non-empty icon and action fields."
                                      << std::endl;
                            return;
                        }

                        const std::string iconPath = ":/icons/" + icon;
                        callbacks.push_back(lc::scripting::nativeCallback(
                            [mWindow, iconPath, operationModuleDirectory,
                             optionName, action]() {
                                mWindow->toolbar()->addButton(
                                    "cancel", iconPath.c_str(),
                                    "Current operation",
                                    lc::scripting::nativeCallback(
                                        [mWindow, action]() {
                                            const auto operation =
                                                mWindow->currentOperation();
                                            if (operation.isNil()) return;
                                            if (!operation.hasAttr(action)) {
                                                mWindow->cliCommand()->write(
                                                    "Python operation has no toolbar action '" +
                                                    action + "'.\n");
                                                return;
                                            }
                                            lc::scripting::ScriptValue result;
                                            std::string error;
                                            if (!operation.callMethodChecked(
                                                    action, result, error)) {
                                                mWindow->cliCommand()->write(
                                                    "Python toolbar action '" +
                                                    action + "' failed: " +
                                                    error + "\n");
                                            }
                                        }),
                                    optionName.c_str(), false, "Quick Access",
                                    operationModuleDirectory.empty()
                                        ? nullptr
                                        : operationModuleDirectory.c_str());
                            }));
                    };

                    auto makeOptionCallbacks = [&](const std::string& scope,
                                                   py::handle rawEntries) {
                        std::vector<lc::scripting::ScriptCallback> callbacks;
                        if (!py::isinstance<py::dict>(rawEntries)) {
                            std::cerr << "[OperationLoader] Python operation '"
                                      << name << "' operation_options['" << scope
                                      << "'] must be a dict."
                                      << std::endl;
                            return callbacks;
                        }
                        const py::dict entries =
                            py::reinterpret_borrow<py::dict>(rawEntries);
                        std::vector<std::string> optionNames;
                        for (const auto item : entries) {
                            if (!py::isinstance<py::str>(item.first)) {
                                std::cerr << "[OperationLoader] Python operation '"
                                          << name << "' option in scope '" << scope
                                          << "' has a non-string label; skipping it."
                                          << std::endl;
                                continue;
                            }
                            optionNames.emplace_back(
                                py::cast<std::string>(item.first));
                        }
                        std::sort(optionNames.begin(), optionNames.end());
                        for (const std::string& optionName : optionNames) {
                            appendOptionCallback(callbacks, optionName,
                                                 entries[py::str(optionName)]);
                        }
                        return callbacks;
                    };

                    std::vector<lc::scripting::ScriptCallback> defaultOptions;
                    for (const std::string& scope : scopeNames) {
                        py::object option = scopes[py::str(scope)];
                        if (scope.find("_init") != std::string::npos) {
                            auto callbacks = makeOptionCallbacks(scope, option);
                            if (!callbacks.empty() && !commandLine.empty()) {
                                mWindow->addOperationOptions(
                                    commandLine + scope,
                                    std::move(callbacks));
                            }
                        } else {
                            appendOptionCallback(defaultOptions, scope, option);
                        }
                    }
                    if (!defaultOptions.empty() && !commandLine.empty()) {
                        mWindow->addOperationOptions(commandLine,
                                                     std::move(defaultOptions));
                    }
                }
            } catch (const py::error_already_set& e) {
                std::cerr << "[OperationLoader] Python operation '"
                          << name << "' operation_options raised: "
                          << e.what() << std::endl;
            }
        }

        // ---- Context menu operation ----
        contextMenuManager->addOperation(name, group);

        // ---- Context transitions ----
        if (py::hasattr(cls, "context_transitions")) {
            py::object transitions = cls.attr("context_transitions");
            if (py::isinstance<py::dict>(transitions)) {
                for (auto item : py::reinterpret_borrow<py::dict>(transitions)) {
                    std::string step = py::str(item.first);
                    std::vector<std::string> transList;
                    if (py::isinstance<py::list>(item.second)) {
                        for (auto t : py::reinterpret_borrow<py::list>(item.second)) {
                            transList.emplace_back(py::str(t));
                        }
                    }
                    contextMenuManager->addTransition(name, step, transList);
                }
            }
        }

        // Operation-option definitions are converted to delayed toolbar
        // callbacks so actions target the active operation instance.
    }
}

#endif  // LC_WITH_PYTHONSCRIPT
