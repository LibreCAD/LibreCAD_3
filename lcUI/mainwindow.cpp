#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogs/aboutdialog.h"
#include "dialogs/textdialog.h"
#include "windowmanager.h"
#include "propertyeditor.h"
#include "managers/contextmenumanager.h"

#include <QStandardPaths>

#include "widgets/guiAPI/coordinategui.h"
#include "widgets/guiAPI/entitygui.h"
#include "widgets/guiAPI/buttongui.h"

// Phase 4 PR-4 — native lambda replacements for the run_script /
// run_customizetoolbar / changeLayout dostrings need the ScriptDock widget.
// Phase 3 PR-3.1 — LuaScript widget renamed to ScriptDock (generalized
// for both Lua and Python).  The Lua-visible name `lc.LuaScript` is
// aliased in guibridge for backward compat.
#include "widgets/scriptdock.h"

// Phase 4 PR-7 — Lua-globals resolver in registerOperationResolvers()
// wraps kaguya::LuaRef via makeLuaObject.
#include <scriptadapter/luacallback.h>

// Phase 5 PR-5.1 fixup — installEventHooks() called unconditionally
// at MainWindow ctor.
#ifdef LC_WITH_PYTHONSCRIPT
#include "python/pyeventhooks.h"
#endif

// Phase 4 PR-9b — trigger* slots build ScriptValue::Map payloads
// natively; CadMdiChild* travels as an OpaquePtr with the tag from
// opaquetags.h.
#include "lua/opaquetags.h"

using namespace lc::ui;

MainWindow::MainWindow()
    :
    ui(new Ui::MainWindow),
    linePatternSelect(&_cadMdiChild, this, true, true),
    lineWidthSelect(_cadMdiChild.metaInfoManager(), this, true, true),
    colorSelect(_cadMdiChild.metaInfoManager(), this, true, true),
    _cliCommand(this),
    _toolbar(&_luaInterface, this),
    _layers(nullptr, this),
    _copyManager(&_cadMdiChild)
{
    ContextMenuManager::GetContextMenuManager(this);
    _contextMenuManagerId = ContextMenuManager::GetInstanceId(this);

    ui->setupUi(this);
    // new document and set mainwindow attributes
    _cadMdiChild.newDocument();
    setWindowTitle(QObject::tr("LibreCAD"));
    setUnifiedTitleAndToolBarOnMac(true);
    setCentralWidget(&_cadMdiChild);

    _layers.setMdiChild(&_cadMdiChild);

    // add widgets to correct positions
    addDockWidget(Qt::RightDockWidgetArea, &_layers);
    addDockWidget(Qt::BottomDockWidgetArea, &_cliCommand);
    addDockWidget(Qt::TopDockWidgetArea, &_toolbar);

    _toolbar.initializeToolbar(&linePatternSelect, &lineWidthSelect, &colorSelect);
    _cadMdiChild.viewer()->autoScale();

    initMenuAPI();

    // connect required signals and slots
    ConnectInputEvents();

    // open qt bridge and run lua scripts
    _luaInterface.initLua(this);

    // Phase 4 PR-7 — register the default Lua-globals resolver so
    // runOperationByName can find operation classes exposed as Lua
    // globals (this is exactly what the killed `run_basic_operation`
    // dostring resolved via `_G[name]`).  Phase 5 will PUSH the Python
    // operation-registry resolver AFTER this — the runOperationByName
    // loop walks the list in reverse so Python takes precedence.
    registerOperationResolver([this](const std::string& name)
                              -> lc::scripting::ScriptObject {
        kaguya::State state(_luaInterface.luaState());
        kaguya::LuaRef ref = state[name];
        if (ref.isNilref() || ref.type() != LUA_TTABLE) {
            return lc::scripting::ScriptObject{};
        }
        return lc::lua::makeLuaObject(std::move(ref));
    });

    // Phase 5 PR-5.1 fixup — install the lc.event.register /
    // lc.event.deregister hooks HERE, not lazily from ScriptDock's
    // ctor.  Python operations registered via lc.event.register at
    // startup (PR-5.2's OperationLoader second source calls a
    // Python operation's `__init__` which registers listeners) fire
    // BEFORE the ScriptDock has ever been opened, so the hook slots
    // were previously still default-constructed (falsy) and every
    // early registration silently no-op'd.
#ifdef LC_WITH_PYTHONSCRIPT
    lc::ui::python::installEventHooks();
#endif

    _toolbar.addSnapOptions();

    addOtherMenus();

    _toolbar.generateButtonsMap();
    readUiSettings();

    _cadMdiChild.viewer()->setContextMenuManagerId(_contextMenuManagerId);

    PropertyEditor* propertyEditor = PropertyEditor::GetPropertyEditor(this);
    this->addDockWidget(Qt::BottomDockWidgetArea, propertyEditor);

    /* Shortcuts */
    copyShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this);
    pasteShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this);

    connect(copyShortcut, &QShortcut::activated, [this]() { this->copySelectedEntities(this->cadMdiChild()->selection()); });
    connect(pasteShortcut, &QShortcut::activated, this, &MainWindow::pasteEvent);

    this->resizeDocks({ &_cliCommand, propertyEditor }, { 65, 35 }, Qt::Horizontal);
}

MainWindow::~MainWindow()
{
    WindowManager::removeWindow(this);
    delete ui;
}

void MainWindow::addOtherMenus() {
    // Phase 4 PR-4 — replaces the phase-4 sub-plan's "Kind C" dostring
    // codegen sites for menu callbacks with native ScriptCallback
    // lambdas.  Kill list (verbatim from the sub-plan / mainwindow.cpp:
    // pre-refactor line 87-88 + 103-111):
    //   * `run_luascript = function() lc.LuaScript(mainWindow):show() end`
    //   * `run_customizetoolbar = function() mainWindow:runCustomizeToolbar() end`
    //   * 5 `changeLayout = function() mainWindow:...() end` sites
    //   * the scratch-global cleanup pattern
    // The precedent for the native lambda replacement is the two Lua-
    // native binding sites already present here at pre-refactor lines
    // 89-96 (`run_aboutdialog`, `run_textdialog`) — the exact same
    // idiom just moved into native callbacks bound directly to menu
    // items via `nativeCallback([]{...})`.

    MainWindow* self = this;

    // Phase 3 PR-3.3 — menu renamed "Lua" → "Script" now that the dock
    // supports both languages.  ScriptDock (phase 3 PR-3.1) replaces
    // the old LuaScript widget; the Lua-visible name `lc.LuaScript`
    // remains bound as an alias in guibridge for backward compat.
    api::Menu* scriptMenu = addMenu("Script");
    scriptMenu->addItem("Run script",
        lc::scripting::nativeCallback([self]() {
            auto ls = new lc::ui::widgets::ScriptDock(self);
            ls->show();
        }));
    scriptMenu->addItem("Customize Toolbar",
        lc::scripting::nativeCallback([self]() {
            self->runCustomizeToolbar();
        }));

    api::Menu* viewMenu = addMenu("View");
    viewMenu->addItem("Default Layout 1",
        lc::scripting::nativeCallback([self]() { self->changeDockLayout(1); }));
    viewMenu->addItem("Default Layout 2",
        lc::scripting::nativeCallback([self]() { self->changeDockLayout(2); }));
    viewMenu->addItem("Default Layout 3",
        lc::scripting::nativeCallback([self]() { self->changeDockLayout(3); }));
    viewMenu->addItem("Load Dock Layout",
        lc::scripting::nativeCallback([self]() { self->loadDockLayout(); }));
    viewMenu->addItem("Save Dock Layout",
        lc::scripting::nativeCallback([self]() { self->saveDockLayout(); }));

    api::Menu* aboutMenu = addMenu("About");
    aboutMenu->addItem("About",
        lc::scripting::nativeCallback([self]() {
            auto aboutDialog = new dialog::AboutDialog(self);
            aboutDialog->show();
        }));

    api::Menu* textMenu = menuByName("Create")->menuByName("Text");
    if (textMenu != nullptr) {
        textMenu->addItem("Text Dialog",
            lc::scripting::nativeCallback([self]() {
                auto textDialog = new dialog::TextDialog(self, self);
                textDialog->show();
            }));
    }
}

void MainWindow::runOperation(lc::scripting::ScriptObject operation,
                              const std::string& init_method) {
    // Phase 4 PR-7 — routed through ScriptObject / ScriptCallback.
    //   * `finish_op` dostring gone → native Cancel-button lambda calling
    //     `_luaInterface.finishOperation()` directly.
    //   * `operation.call<LuaRef>()` → `operation.instantiate()`, which
    //     preserves the instance's method-callable identity (unlike the
    //     lossy Value round-trip that `call()` does).
    //   * `op["_init_default"](op)` → `instance.callMethod("_init_default")`
    //     (the Lua adapter prepends `self` implicitly — see luacallback.cpp).
    _cliCommand.setFocus();
    _luaInterface.finishOperation();
    _cadMdiChild.viewer()->setOperationActive(true);

    // Extra option toolbar icons.  Two-tier key: first "cmdLine+init",
    // then plain "cmdLine".  Matches the legacy two-branch lookup.
    if (operation.hasAttr("operation_options")) {
        std::string cmdLine = operation.getAttr("command_line").asString();
        auto it = operation_options.find(cmdLine + init_method);
        if (it == operation_options.end()) {
            it = operation_options.find(cmdLine);
        }
        if (it != operation_options.end()) {
            for (auto& opt : it->second) {
                // Fire the option: it adds an icon button to the toolbar.
                opt.invoke();
            }
        }
    }

    // Cancel button — was `finish_op = function() finish_operation() end`
    // dostring (killed in PR-7).  Native lambda calls
    // `luaInterface.finishOperation()` directly.
    _toolbar.addButton("", ":/icons/quit.svg", "Current operation",
        lc::scripting::nativeCallback([this]() {
            _luaInterface.finishOperation();
        }),
        "Cancel");

    // Instantiate the operation class + run its init method.
    lc::scripting::ScriptObject instance = operation.instantiate();
    _luaInterface.setOperation(instance);
    if (init_method.empty()) {
        if (instance.hasAttr("_init_default")) {
            instance.callMethod("_init_default");
        }
    } else {
        instance.callMethod(init_method);
    }

    _oldOperation = std::move(operation);
    _oldOpInitMethod = init_method;
}

void MainWindow::runOperationByName(const std::string& name,
                                    const std::string& init_method) {
    // Phase 4 PR-7 — native replacement for the Lua-side
    // `run_basic_operation(name, init_method)` dostring family (killed
    // in operationloader.cpp).  Ordered resolver list: latest-registered
    // wins so Python registry (phase 5) supersedes the Lua-globals
    // resolver.
    for (auto it = _operationResolvers.rbegin();
         it != _operationResolvers.rend(); ++it) {
        auto op = (*it)(name);
        if (!op.isNil()) {
            runOperation(std::move(op), init_method);
            return;
        }
    }
    // Unresolved name: silently no-op, matching Lua's dostring behavior
    // where `run_basic_operation(nil)` produced no-op via LuaObjectImpl's
    // nil-guard.  A future PR may surface a diagnostic through the CLI.
}

void MainWindow::registerOperationResolver(OperationResolver resolver) {
    _operationResolvers.push_back(std::move(resolver));
}

void MainWindow::addOperationOptions(std::string operation,
                                     std::vector<lc::scripting::ScriptCallback> options) {
    operation_options[std::move(operation)] = std::move(options);
}

void MainWindow::operationFinished() {
    // remove operation group
    _toolbar.removeGroupByName("Current operation");
    _cadMdiChild.viewer()->setOperationActive(false);
}

lc::ui::widgets::CliCommand* MainWindow::cliCommand() {
    return &_cliCommand;
}

lc::ui::CadMdiChild* MainWindow::cadMdiChild() {
    return &_cadMdiChild;
}

lc::ui::widgets::Toolbar* MainWindow::toolbar() {
    return &_toolbar;
}

lc::ui::widgets::Layers* MainWindow::layers() {
    return &_layers;
}

lc::ui::LuaInterface* MainWindow::luaInterface() {
    return &_luaInterface;
}

int MainWindow::contextMenuManagerId() {
    return _contextMenuManagerId;
}

void MainWindow::ConnectInputEvents()
{
    // CadMdiChild connections, main window should not know about proxy
    QObject::connect(&_cadMdiChild, &CadMdiChild::mousePressEvent, this, &MainWindow::triggerMousePressed);
    QObject::connect(&_cadMdiChild, &CadMdiChild::mouseReleaseEvent, this, &MainWindow::triggerMouseReleased);
    QObject::connect(&_cadMdiChild, &CadMdiChild::mouseMoveEvent, this, &MainWindow::triggerMouseMoved);
    QObject::connect(&_cadMdiChild, &CadMdiChild::selectionChangeEvent, this, &MainWindow::triggerSelectionChanged);
    QObject::connect(&_cadMdiChild, &CadMdiChild::selectionChangeEvent, this, &MainWindow::selectionChanged);
    QObject::connect(&_cadMdiChild, &CadMdiChild::keyPressEventx, this, &MainWindow::triggerKeyPressed);
    QObject::connect(&_cadMdiChild, &CadMdiChild::keyPressed, &_cliCommand, &widgets::CliCommand::onKeyPressed);

    // CliCommand connections
    QObject::connect(&_cliCommand, &widgets::CliCommand::coordinateEntered, this, &MainWindow::triggerCoordinateEntered);
    QObject::connect(&_cliCommand, &widgets::CliCommand::relativeCoordinateEntered, this, &MainWindow::triggerRelativeCoordinateEntered);
    QObject::connect(&_cliCommand, &widgets::CliCommand::numberEntered, this, &MainWindow::triggerNumberEntered);
    QObject::connect(&_cliCommand, &widgets::CliCommand::textEntered, this, &MainWindow::triggerTextEntered);
    QObject::connect(&_cliCommand, &widgets::CliCommand::finishOperation, this, &MainWindow::triggerFinishOperation);
    QObject::connect(&_cliCommand, &widgets::CliCommand::commandEntered, this, &MainWindow::triggerCommandEntered);

    // Layers to select tools connections
    QObject::connect(&_layers, &widgets::Layers::layerChanged, &linePatternSelect, &widgets::LinePatternSelect::onLayerChanged);
    QObject::connect(&_layers, &widgets::Layers::layerChanged, &lineWidthSelect, &widgets::LineWidthSelect::onLayerChanged);
    QObject::connect(&_layers, &widgets::Layers::layerChanged, &colorSelect, &widgets::ColorSelect::onLayerChanged);

    // Other
    QObject::connect(this, &MainWindow::point, this, &MainWindow::triggerPoint);
    QObject::connect(findMenuItemByObjectName("actionExit"), &QAction::triggered, this, &MainWindow::close);

    // File connections
    QObject::connect(findMenuItemByObjectName("actionNew"), &QAction::triggered, this, &MainWindow::newFile);
    QObject::connect(findMenuItemByObjectName("actionOpen"), &QAction::triggered, this, &MainWindow::openFile);
    QObject::connect(findMenuItemByObjectName("actionSave_2"), &QAction::triggered, &_cadMdiChild, &CadMdiChild::saveFile);
    QObject::connect(findMenuItemByObjectName("actionSave_As"), &QAction::triggered, &_cadMdiChild, &CadMdiChild::saveAsFile);

    // Edit connections
    QObject::connect(findMenuItemByObjectName("actionUndo"), &QAction::triggered, this, &MainWindow::undo);
    QObject::connect(findMenuItemByObjectName("actionRedo"), &QAction::triggered, this, &MainWindow::redo);
    QObject::connect(findMenuItemByObjectName("actionSelect_All"), &QAction::triggered, this, &MainWindow::selectAll);
    QObject::connect(findMenuItemByObjectName("actionSelect_None"), &QAction::triggered, this, &MainWindow::selectNone);
    QObject::connect(findMenuItemByObjectName("actionInvert_Selection"), &QAction::triggered, this, &MainWindow::invertSelection);
    QObject::connect(findMenuItemByObjectName("actionClear_Undoable_Stack"), &QAction::triggered, this, &MainWindow::clearUndoableStack);
    QObject::connect(findMenuItemByObjectName("actionAuto_Scale"), &QAction::triggered, this, &MainWindow::autoScale);
}

void MainWindow::runLastOperation() {
    // Phase 4 PR-7 — ScriptObject nil check.
    if (!_oldOperation.isNil()) {
        runOperation(_oldOperation, _oldOpInitMethod);
    }
}

/* Menu functions */

void MainWindow::connectMenuItem(const std::string& itemName,
                                 lc::scripting::ScriptCallback callback)
{
    lc::ui::api::MenuItem* menuItem = findMenuItemByObjectName(itemName.c_str());
    menuItem->addCallback(std::move(callback));
}

void MainWindow::initMenuAPI() {
    QList<QMenu*> allMenus = menuBar()->findChildren<QMenu*>(QString(), Qt::FindDirectChildrenOnly);

    int menuPosition = 0;
    for (QMenu* current_menu : allMenus)
    {
        api::Menu* menu = static_cast<api::Menu*>(current_menu);
        this->menuBar()->addAction(menu->menuAction());
        menuMap[menu->title()] = menu;
        menu->updatePositionVariable(menuPosition);
        menuPosition++;

        QList<QMenu*> allMenusOfCurrentMenu = menu->findChildren<QMenu*>(QString(), Qt::FindDirectChildrenOnly);

        for (QMenu* currentChildMenu : allMenusOfCurrentMenu)
        {
            if (currentChildMenu != nullptr) {
                menu->addAction(currentChildMenu->menuAction());
            }
        }

        addActionsAsMenuItem(menu);
        fixMenuPositioning(menu);
    }
}

void MainWindow::addActionsAsMenuItem(lc::ui::api::Menu* menu) {
    QList<QAction*> actions = menu->actions();
    QList<QAction*> menuItemsToBeAdded;

    for (QAction* action : actions)
    {
        if (action->menu()) {
            addActionsAsMenuItem(static_cast<api::Menu*>(action->menu()));
        }
        else if (action->isSeparator()) {
            QAction* sep = new QAction();
            sep->setSeparator(true);
            menu->removeAction(action);
            menuItemsToBeAdded.push_back(sep);
        }
        else {
            lc::ui::api::MenuItem* newMenuItem = new lc::ui::api::MenuItem(action->text().toStdString().c_str());

            QString oldObjectName = action->objectName();
            action->setObjectName(oldObjectName + QString("changed"));
            newMenuItem->setObjectName(oldObjectName);

            menu->removeAction(action);
            menuItemsToBeAdded.push_back(newMenuItem);
        }
    }

    for (QAction* it : menuItemsToBeAdded)
    {
        menu->addAction(it);
    }

    // reorder menu to appear below
    for (QAction* action : actions)
    {
        if (action->menu()) {
            menu->insertMenu(menuItemsToBeAdded.last(), action->menu());
        }
    }
}

void MainWindow::fixMenuPositioning(lc::ui::api::Menu* menu) {
    QList<QAction*> actions = menu->actions();

    int pos = 0;
    for (QAction* action : actions) {
        lc::ui::api::Menu* actionMenu = qobject_cast<lc::ui::api::Menu*>(action->menu());
        lc::ui::api::MenuItem* actionItem = qobject_cast<lc::ui::api::MenuItem*>(action);

        if (actionMenu != nullptr) {
            actionMenu->updatePositionVariable(pos);
        }

        if (actionItem != nullptr) {
            actionItem->updatePositionVariable(pos);
        }
        pos++;
    }
}

/* Menu Lua GUI API */

lc::ui::api::MenuItem* MainWindow::findMenuItem(std::string label) {
    return findMenuItemBy(label, true);
}

lc::ui::api::MenuItem* MainWindow::findMenuItemByObjectName(std::string objectName) {
    return findMenuItemBy(objectName, false);
}

api::MenuItem* MainWindow::findMenuItemBy(std::string objectName, bool searchByLabel) {
    QString name = QString(objectName.c_str());

    for (auto key : menuMap.keys())
    {
        api::MenuItem* foundIt = findMenuItemRecur(menuMap[key], name, searchByLabel);

        if (foundIt != nullptr) {
            return foundIt;
        }
    }

    return nullptr;
}

api::MenuItem* MainWindow::findMenuItemRecur(QMenu* menu, QString objectName, bool searchByLabel) {
    QList<QAction*> actions = menu->actions();

    for (QAction* action : actions)
    {
        if (action->menu()) {
            api::MenuItem* foundIt = findMenuItemRecur(action->menu(), objectName, searchByLabel);
            if (foundIt != nullptr) {
                return foundIt;
            }
        }
        else if (!action->isSeparator()) {
            if (searchByLabel) {
                if (objectName == action->text()) {
                    return static_cast<api::MenuItem*>(action);
                }
            }
            else {
                if (objectName == action->objectName()) {
                    return static_cast<api::MenuItem*>(action);
                }
            }
        }
    }

    return nullptr;
}

bool MainWindow::checkForMenuOfSameLabel(const std::string& label) {
    QList<QString> keys = menuMap.keys();

    for (QString key : keys)
    {
        std::string keystr = key.toStdString();
        keystr.erase(std::remove(keystr.begin(), keystr.end(), '&'), keystr.end());

        if (keystr == label) {
            return true;
        }
    }

    return false;
}

api::Menu* MainWindow::addMenu(const std::string& menuName) {
    if (checkForMenuOfSameLabel(menuName)) {
        return nullptr;
    }

    api::Menu* newMenu = new api::Menu(menuName.c_str());
    addMenu(newMenu);

    return newMenu;
}

void MainWindow::addMenu(lc::ui::api::Menu* menu) {
    if (checkForMenuOfSameLabel(menu->label())) {
        return;
    }

    menuMap[menu->title()] = menu;
    menuBar()->addMenu(menu);

    QList<QAction*> menuList = menuBar()->actions();
    menu->updatePositionVariable(menuList.size() - 1);
}

api::Menu* MainWindow::menuByName(const std::string& menuName) {
    QList<QString> keys = menuMap.keys();

    for (QString key : keys)
    {
        std::string keystr = key.toStdString();
        keystr.erase(std::remove(keystr.begin(), keystr.end(), '&'), keystr.end());

        if (keystr == menuName) {
            return menuMap[key];
        }
    }

    return nullptr;
}

lc::ui::api::Menu* MainWindow::menuByPosition(int pos) {
    QList<QAction*> menuList = menuBar()->actions();

    if (pos < 0 || pos >= menuList.size()) {
        return nullptr;
    }

    return dynamic_cast<lc::ui::api::Menu*>(menuList[pos]->menu());
}

void MainWindow::removeFromMenuMap(std::string menuName) {
    auto iter = menuMap.begin();
    QString key;
    for(; iter != menuMap.end(); ++iter)
    {
        std::string keystr = iter.key().toStdString();
        keystr.erase(std::remove(keystr.begin(), keystr.end(), '&'), keystr.end());

        if (keystr == menuName) {
            key = iter.key();
            break;
        }
    }

    menuMap.remove(key);
}

void MainWindow::removeMenu(const char* menuLabel) {
    lc::ui::api::Menu* menuremove = menuByName(menuLabel);
    if (menuremove != nullptr) {
        menuremove->remove();
    }
}

void MainWindow::removeMenu(int position) {
    lc::ui::api::Menu* menuremove = menuByPosition(position);
    if (menuremove != nullptr) {
        menuremove->remove();
    }
}

/* Trigger slots */

// Phase 4 PR-9b — every trigger* slot now builds a ScriptValue::Map
// payload natively and hits `LuaInterface::triggerEvent(string,
// ScriptValue)` (PR-9a's native overload) — no more Lua scratch globals
// (`mousePressed`, `keyEvent`, `numberEntered`, ...) manufactured on
// every event.  The `widget` entry (previously `state[...]["widget"] =
// &_cadMdiChild`) becomes an OpaquePtr with the `CadMdiChild*` tag; the
// adapter's registered encoder materializes it back into userdata on
// entry to each Lua listener.
//
// Lazy materialization: check `listenerCount(name) > 0` before building
// expensive payloads — mouseMove fires per pixel, and manufacturing a
// per-event Map only to have zero listeners see it is measurable waste.
//
// The parallel Qt `emit point(...)` signal calls in triggerMousePressed
// / triggerCoordinateEntered / triggerRelativeCoordinateEntered are
// untouched — they feed the C++-side `triggerPoint` slot that updates
// `lastPoint`.  Preserving them verbatim is part of PR-9b's contract.

namespace {
    // Build a ScriptValue holding an OpaquePtr(CadMdiChild*).  Named
    // helper so every trigger slot's `widget` entry reads the same.
    inline lc::scripting::ScriptValue widgetOpaque(lc::ui::CadMdiChild* w) {
        return lc::scripting::ScriptValue(
            lc::scripting::OpaquePtr{w, lc::ui::opaquetag::CadMdiChild});
    }
}

void MainWindow::triggerMousePressed()
{
    lc::geo::Coordinate cursorPos = _cadMdiChild.cursor()->position();
    if (_luaInterface.listenerCount("point") > 0) {
        auto payload = lc::scripting::makeMap();
        (*payload)["position"] = lc::scripting::ScriptValue(cursorPos);
        (*payload)["widget"]   = widgetOpaque(&_cadMdiChild);
        _luaInterface.triggerEvent("point", lc::scripting::ScriptValue(payload));
    }

    emit point(cursorPos);
}

void MainWindow::triggerMouseReleased()
{
    if (_luaInterface.listenerCount("mouseRelease") == 0) return;
    auto payload = lc::scripting::makeMap();
    (*payload)["widget"] = widgetOpaque(&_cadMdiChild);
    _luaInterface.triggerEvent("mouseRelease", lc::scripting::ScriptValue(payload));
}

void MainWindow::triggerSelectionChanged()
{
    if (_luaInterface.listenerCount("selectionChanged") == 0) return;
    auto payload = lc::scripting::makeMap();
    (*payload)["widget"] = widgetOpaque(&_cadMdiChild);
    _luaInterface.triggerEvent("selectionChanged", lc::scripting::ScriptValue(payload));
}

void MainWindow::triggerMouseMoved()
{
    // mouseMove is the hottest per-pixel event; the listenerCount guard
    // is not a micro-optimization here, it's the observable difference
    // between "adds ~1 payload construction per pixel of movement" and
    // "adds nothing when no script is listening".
    if (_luaInterface.listenerCount("mouseMove") == 0) return;
    lc::geo::Coordinate cursorPos = _cadMdiChild.cursor()->position();
    auto payload = lc::scripting::makeMap();
    (*payload)["position"] = lc::scripting::ScriptValue(cursorPos);
    (*payload)["widget"]   = widgetOpaque(&_cadMdiChild);
    _luaInterface.triggerEvent("mouseMove", lc::scripting::ScriptValue(payload));
}

void MainWindow::triggerKeyPressed(int key)
{
    if (key == Qt::Key_Escape)
    {
        // Escape triggers the finishOperation event with an empty
        // payload (was `kaguya::LuaRef(state)` — a nil LuaRef; the
        // neutral form uses default-constructed ScriptValue).
        _luaInterface.triggerEvent("finishOperation");
    }
    else
    {
        if (_luaInterface.listenerCount("keyPressed") == 0) return;
        auto payload = lc::scripting::makeMap();
        (*payload)["key"]    = lc::scripting::ScriptValue(key);
        (*payload)["widget"] = widgetOpaque(&_cadMdiChild);
        _luaInterface.triggerEvent("keyPressed", lc::scripting::ScriptValue(payload));
    }
}

void MainWindow::triggerCoordinateEntered(lc::geo::Coordinate coordinate)
{
    if (_luaInterface.listenerCount("point") > 0) {
        auto payload = lc::scripting::makeMap();
        (*payload)["position"] = lc::scripting::ScriptValue(coordinate);
        (*payload)["widget"]   = widgetOpaque(&_cadMdiChild);
        _luaInterface.triggerEvent("point", lc::scripting::ScriptValue(payload));
    }

    emit point(coordinate);
}

void MainWindow::triggerRelativeCoordinateEntered(lc::geo::Coordinate coordinate)
{
    if (_luaInterface.listenerCount("point") > 0) {
        auto payload = lc::scripting::makeMap();
        (*payload)["position"] =
            lc::scripting::ScriptValue(lastPoint + coordinate);
        (*payload)["widget"] = widgetOpaque(&_cadMdiChild);
        _luaInterface.triggerEvent("point", lc::scripting::ScriptValue(payload));
    }

    emit point(lastPoint + coordinate);
}

void MainWindow::triggerNumberEntered(double number)
{
    if (_luaInterface.listenerCount("number") == 0) return;
    auto payload = lc::scripting::makeMap();
    (*payload)["number"] = lc::scripting::ScriptValue(number);
    (*payload)["widget"] = widgetOpaque(&_cadMdiChild);
    _luaInterface.triggerEvent("number", lc::scripting::ScriptValue(payload));
}

void MainWindow::triggerTextEntered(QString text)
{
    if (_luaInterface.listenerCount("text") == 0) return;
    auto payload = lc::scripting::makeMap();
    (*payload)["text"]   = lc::scripting::ScriptValue(text.toStdString());
    (*payload)["widget"] = widgetOpaque(&_cadMdiChild);
    _luaInterface.triggerEvent("text", lc::scripting::ScriptValue(payload));
}

void MainWindow::triggerFinishOperation()
{
    // Double-fire order preserved verbatim: operationFinished THEN
    // finishOperation (mainwindow.cpp:598-599 pre-refactor, per the
    // phase-4 sub-plan's PRESERVE list).  Both use empty payloads
    // (was `kaguya::LuaRef(state)` — a nil LuaRef).
    _luaInterface.triggerEvent("operationFinished");
    _luaInterface.triggerEvent("finishOperation");
}

void MainWindow::triggerCommandEntered(QString command)
{
    _cliCommand.runCommand(command.toStdString().c_str());
}

void MainWindow::triggerPoint(lc::geo::Coordinate coordinate)
{
    lastPoint = coordinate;
}

void MainWindow::newFile()
{
    /*
        TODO : Ask user if he wishes to save the file before replacing current window with new file
    */

    WindowManager::newFile(this);
}

void MainWindow::openFile()
{
    WindowManager::openFile();
}

// Edit slots
void MainWindow::undo()
{
    _cadMdiChild.undoManager()->undo();
    _cadMdiChild.viewer()->update();
}

void MainWindow::clearUndoableStack()
{
    _cadMdiChild.undoManager()->removeUndoables();
}

void MainWindow::redo()
{
    _cadMdiChild.undoManager()->redo();
    _cadMdiChild.viewer()->update();
}

void MainWindow::selectAll()
{
    _cadMdiChild.viewer()->docCanvas()->selectAll();
    _cadMdiChild.viewer()->update();
}

void MainWindow::selectNone()
{
    _cadMdiChild.viewer()->docCanvas()->removeSelection();
    _cadMdiChild.viewer()->update();
}

void MainWindow::invertSelection()
{
    _cadMdiChild.viewer()->docCanvas()->inverseSelection();
    _cadMdiChild.viewer()->update();
}

void MainWindow::autoScale() {
    _cadMdiChild.viewer()->autoScale();
    _cadMdiChild.viewer()->update();
};

void MainWindow::runCustomizeToolbar() {
    _customizeToolbar = new widgets::CustomizeToolbar(toolbar());
    connect(_customizeToolbar, &widgets::CustomizeToolbar::customizeWidgetClosed, this, &MainWindow::writeSettings);
    connect(_customizeToolbar, &widgets::CustomizeToolbar::defaultSettingsLoad, this, &MainWindow::loadDefaultSettings);

    _customizeToolbar->show();
}

void MainWindow::writeSettings() {
    _uiSettings.writeSettings(_customizeToolbar);
}

void MainWindow::readUiSettings() {
    _customizeToolbar = new widgets::CustomizeToolbar(toolbar());
    _customizeToolbar->setCloseMode(widgets::CustomizeToolbar::CloseMode::Save);
    _uiSettings.readSettings(_customizeToolbar);
    _customizeToolbar->close();
}

void MainWindow::loadDefaultSettings() {
    _uiSettings.readSettings(_customizeToolbar, true);
}

void MainWindow::selectionChanged() {
    std::vector<lc::entity::CADEntity_CSPtr> selectedEntities = _cadMdiChild.selection();
    PropertyEditor* propertyEditor = PropertyEditor::GetPropertyEditor(this);

    propertyEditor->clear(selectedEntities);

    for (lc::entity::CADEntity_CSPtr selectedEntity : selectedEntities) {
        propertyEditor->addEntity(selectedEntity);
    }

    if (selectedEntities.size() == 0) {
        propertyEditor->hide();
    }
    else {
        propertyEditor->show();
    }
}

std::string MainWindow::lastOperationName() {
    // Phase 4 PR-7 — was `_oldOperation["name"].get<std::string>()`.
    return _oldOperation.getAttr("name").asString();
}

lc::scripting::ScriptObject MainWindow::currentOperation() {
    return _luaInterface.operation();
}

void MainWindow::changeDockLayout(int i) {
    if (i == 1) {
        addDockWidget(Qt::RightDockWidgetArea, &_layers);
        addDockWidget(Qt::BottomDockWidgetArea, &_cliCommand);
        addDockWidget(Qt::TopDockWidgetArea, &_toolbar);
        PropertyEditor* propertyEditor = PropertyEditor::GetPropertyEditor(this);
        addDockWidget(Qt::BottomDockWidgetArea, propertyEditor);
        resizeDocks({ &_cliCommand, propertyEditor }, { 65, 35 }, Qt::Horizontal);
    }

    if (i == 2) {
        addDockWidget(Qt::LeftDockWidgetArea, &_layers);
        addDockWidget(Qt::BottomDockWidgetArea, &_cliCommand);
        addDockWidget(Qt::TopDockWidgetArea, &_toolbar);
        PropertyEditor* propertyEditor = PropertyEditor::GetPropertyEditor(this);
        addDockWidget(Qt::RightDockWidgetArea, propertyEditor);
    }

    if (i == 3) {
        addDockWidget(Qt::LeftDockWidgetArea, &_layers);
        addDockWidget(Qt::BottomDockWidgetArea, &_cliCommand);
        addDockWidget(Qt::TopDockWidgetArea, &_toolbar);
        PropertyEditor* propertyEditor = PropertyEditor::GetPropertyEditor(this);
        addDockWidget(Qt::BottomDockWidgetArea, propertyEditor);
    }
}

void MainWindow::copySelectedEntities(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities) {
    _copyManager.copyEntitiesToClipboard(cadEntities);
}

void MainWindow::pasteEvent() {
    _copyManager.pasteEvent();
}

void MainWindow::saveDockLayout() {
    int layersPos = this->dockWidgetArea(&_layers);
    int cliCommandPos = this->dockWidgetArea(&_cliCommand);
    int toolbarPos = this->dockWidgetArea(&_toolbar);
    PropertyEditor* propertyEditor = PropertyEditor::GetPropertyEditor(this);
    int propertyEditorPos = this->dockWidgetArea(propertyEditor);

    std::map<std::string, int> positions = {
        {"Toolbar", toolbarPos},
        {"CliCommand", cliCommandPos},
        {"Layers", layersPos},
        {"PropertyEditor", propertyEditorPos}
    };

    std::map<std::string, int> widths = {
        {"Toolbar", _toolbar.width()},
        {"CliCommand", _cliCommand.width()},
        {"Layers", _layers.width()},
        {"PropertyEditor", propertyEditor->width()}
    };

    std::map<int, int> posCount;
    std::map<int, int> posWidth;
    for (auto iter = positions.begin(); iter != positions.end(); ++iter) {
        if (posCount.find(iter->second) == posCount.end()) {
            posCount[iter->second] = 0;
            posWidth[iter->second] = 0;
        }
        posCount[iter->second]++;
        posWidth[iter->second] += widths[iter->first];
    }

    std::map<std::string, int> posProportions;

    for (auto iter = positions.begin(); iter != positions.end(); ++iter) {
        if (posCount[iter->second] > 1) {
            int percent = std::round(((double)widths[iter->first] / (double)posWidth[iter->second]) * 100);
            posProportions[iter->first] = percent;
        }
    }

    _uiSettings.writeDockSettings(positions, posProportions);
}

void MainWindow::loadDockLayout() {
    std::map<std::string, int> dockProportions;
    std::map<std::string, int> dockPositions = _uiSettings.readDockSettings(dockProportions);

    PropertyEditor* propertyEditor = PropertyEditor::GetPropertyEditor(this);
    std::map<std::string, QDockWidget*> dockWidgets = {
        {"Layers", &_layers},
        {"CliCommand", &_cliCommand},
        {"Toolbar", &_toolbar},
        {"PropertyEditor", propertyEditor}
    };

    std::map<int, QList<QDockWidget*>> resizeWidgets;
    std::map<int, QList<int>> resizeProportions;

    if (dockPositions.size() > 0) {
        for (auto iter = dockWidgets.begin(); iter != dockWidgets.end(); ++iter) {
            int pos = dockPositions[iter->first];
            addDockWidget((Qt::DockWidgetArea)pos, iter->second);

            if (dockProportions.find(iter->first) != dockProportions.end()) {
                if (resizeWidgets.find(pos) == resizeWidgets.end()) {
                    resizeWidgets[pos] = QList<QDockWidget*>();
                    resizeProportions[pos] = QList<int>();
                }

                resizeWidgets[pos].append(iter->second);
                resizeProportions[pos].append(dockProportions[iter->first]);
            }
        }

        for (auto iter = resizeWidgets.begin(); iter != resizeWidgets.end(); ++iter) {
            bool horiz = true;
            if ((iter->first == (int)Qt::LeftDockWidgetArea) || (iter->first == (int)Qt::RightDockWidgetArea)) {
                horiz = false;
            }
            resizeDocks(iter->second, resizeProportions[iter->first], (horiz) ? Qt::Horizontal : Qt::Vertical);
        }
    }
}
