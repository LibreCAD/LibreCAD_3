#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogs/aboutdialog.h"
#include "windowmanager.h"

using namespace lc::ui;

MainWindow::MainWindow()
    :
    ui(new Ui::MainWindow),
    linePatternSelect(&_cadMdiChild, this, true, true),
    lineWidthSelect(_cadMdiChild.metaInfoManager(), this, true, true),
    colorSelect(_cadMdiChild.metaInfoManager(), this, true, true),
    _cliCommand(this),
    _toolbar(&_luaInterface, this)
{
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

    _toolbar.addSnapOptions();

    // add lua script
    kaguya::State state(_luaInterface.luaState());
    state.dostring("run_luascript = function() lc.LuaScript(mainWindow):show() end");
    state.dostring("run_customizetoolbar = function() mainWindow:runCustomizeToolbar() end");
    state["run_aboutdialog"] = kaguya::function([&]{
	    auto aboutDialog = new dialog::AboutDialog(this);
	    aboutDialog->show();
    });

    api::Menu* luaMenu = addMenu("Lua");
    luaMenu->addItem("Run script", state["run_luascript"]);
    luaMenu->addItem("Customize Toolbar", state["run_customizetoolbar"]);

    api::Menu* aboutMenu = addMenu("About");
    aboutMenu->addItem("About", state["run_aboutdialog"]);

    _toolbar.generateButtonsMap();
    readUiSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::runOperation(kaguya::LuaRef operation, const std::string& init_method){
    _cliCommand.setFocus();
    _luaInterface.finishOperation();
    kaguya::State state(_luaInterface.luaState());

    // if current operation had extra operation _toolbar icons, add them
    if (!operation["operation_options"].isNilref())
    {
        if (operation_options.find(operation["command_line"].get<std::string>() + init_method) != operation_options.end()) {
            std::vector<kaguya::LuaRef>& options = operation_options[operation["command_line"].get<std::string>() + init_method];

            for (auto op : options) {
                // run operation which adds option icon to _toolbar
                op();
            }
        }else if (operation_options.find(operation["command_line"]) != operation_options.end()) {
            std::vector<kaguya::LuaRef>& options = operation_options[operation["command_line"]];

            for (auto op : options) {
                // run operation which adds option icon to _toolbar
                op();
            }
        }
    }

    // add _toolbar cancel button
    state.dostring("finish_op = function() finish_operation() end");
    _toolbar.addButton("", ":/icons/quit.svg", "Current operation", state["finish_op"], "Cancel");
    state["finish_op"] = nullptr;

    // call operation to run CreateOperations init method etc
    _luaInterface.setOperation(operation.call<kaguya::LuaRef>());
    kaguya::LuaRef op = _luaInterface.operation();
    if (init_method == "") {
        if (!op["_init_default"].isNilref()) {
            op["_init_default"](op);
        }
    }
    else {
        op[init_method.c_str()](op);
    }
}

void MainWindow::addOperationOptions(std::string operation, std::vector<kaguya::LuaRef> options) {
    operation_options[operation] = options;
}

void MainWindow::operationFinished() {
    // remove operation group
    _toolbar.removeGroupByName("Current operation");
    _cadMdiChild.viewer()->setOperationActive(false);
}

lc::ui::widgets::CliCommand* MainWindow::cliCommand(){
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

void MainWindow::ConnectInputEvents()
{   
    // CadMdiChild connections
    QObject::connect(_cadMdiChild.viewerProxy(), &LCADViewerProxy::mousePressEvent, this, &MainWindow::triggerMousePressed);
    QObject::connect(_cadMdiChild.viewerProxy(), &LCADViewerProxy::mouseReleaseEvent, this, &MainWindow::triggerMouseReleased);
    QObject::connect(_cadMdiChild.viewerProxy(), &LCADViewerProxy::mouseMoveEvent, this, &MainWindow::triggerMouseMoved);
    QObject::connect(_cadMdiChild.viewerProxy(), &LCADViewerProxy::keyPressEvent, this, &MainWindow::triggerKeyPressed);
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
}

/* Menu functions */

void MainWindow::connectMenuItem(const std::string& itemName, kaguya::LuaRef callback)
{
    lc::ui::api::MenuItem* menuItem = findMenuItemByObjectName(itemName.c_str());
    menuItem->addCallback(callback);
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
        else{
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
            else{ 
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

void MainWindow::triggerMousePressed()
{
    lc::geo::Coordinate cursorPos = _cadMdiChild.cursor()->position();
    kaguya::State state(_luaInterface.luaState());
    state["mousePressed"] = kaguya::NewTable();
    state["mousePressed"]["position"] = cursorPos;
    state["mousePressed"]["widget"] = &_cadMdiChild;
    _luaInterface.triggerEvent("point", state["mousePressed"]);

    emit point(cursorPos);
}

void MainWindow::triggerMouseReleased()
{
    kaguya::State state(_luaInterface.luaState());
    state["mouseRelease"] = kaguya::NewTable();
    state["mouseRelease"]["widget"] = &_cadMdiChild;
    _luaInterface.triggerEvent("selectionChanged", state["mouseRelease"]);
}

void MainWindow::triggerMouseMoved()
{
    lc::geo::Coordinate cursorPos = _cadMdiChild.cursor()->position();
    kaguya::State state(_luaInterface.luaState());
    state["mouseMove"] = kaguya::NewTable();
    state["mouseMove"]["position"] = cursorPos;
    state["mouseMove"]["widget"] = &_cadMdiChild;
    _luaInterface.triggerEvent("mouseMove", state["mouseMove"]);
}

void MainWindow::triggerKeyPressed(int key)
{
    if (key == Qt::Key_Escape)
    {
        // run finish operation
        auto state = _luaInterface.luaState();
        _luaInterface.triggerEvent("finishOperation", kaguya::LuaRef(state));
    }
    else
    {
        kaguya::State state(_luaInterface.luaState());
        state["keyEvent"] = kaguya::NewTable();
        state["keyEvent"]["key"] = key;
        state["keyEvent"]["widget"] = &_cadMdiChild;
        _luaInterface.triggerEvent("keyPressed", state["keyEvent"]);
    }
}

void MainWindow::triggerCoordinateEntered(lc::geo::Coordinate coordinate)
{
    kaguya::State state(_luaInterface.luaState());
    state["coordinateEntered"] = kaguya::NewTable();
    state["coordinateEntered"]["position"] = coordinate;
    state["coordinateEntered"]["widget"] = &_cadMdiChild;
    _luaInterface.triggerEvent("point", state["coordinateEntered"]);

    emit point(coordinate);
}

void MainWindow::triggerRelativeCoordinateEntered(lc::geo::Coordinate coordinate)
{
    kaguya::State state(_luaInterface.luaState());
    state["relCoordinateEntered"] = kaguya::NewTable();
    state["relCoordinateEntered"]["position"] = lastPoint + coordinate;
    state["relCoordinateEntered"]["widget"] = &_cadMdiChild;
    _luaInterface.triggerEvent("point", state["relCoordinateEntered"]);

    emit point(lastPoint + coordinate);
}

void MainWindow::triggerNumberEntered(double number)
{
    kaguya::State state(_luaInterface.luaState());
    state["numberEntered"] = kaguya::NewTable();
    state["numberEntered"]["number"] = number;
    state["numberEntered"]["widget"] = &_cadMdiChild;
    _luaInterface.triggerEvent("number", state["numberEntered"]);
}

void MainWindow::triggerTextEntered(QString text)
{
    kaguya::State state(_luaInterface.luaState());
    state["textEntered"] = kaguya::NewTable();
    state["textEntered"]["text"] = text;
    state["textEntered"]["widget"] = &_cadMdiChild;
    _luaInterface.triggerEvent("text", state["textEntered"]);
}

void MainWindow::triggerFinishOperation()
{
    auto state = _luaInterface.luaState();
    _luaInterface.triggerEvent("operationFinished", kaguya::LuaRef(state));
    _luaInterface.triggerEvent("finishOperation", kaguya::LuaRef(state));
}

void MainWindow::triggerCommandEntered(QString command)
{
    _cliCommand.runCommand(command.toStdString().c_str());
}

void MainWindow::triggerPoint(lc::geo::Coordinate coordinate)
{
    lastPoint = coordinate;
    _cadMdiChild.viewer()->docCanvas()->selectPoint(coordinate.x(), coordinate.y());
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
