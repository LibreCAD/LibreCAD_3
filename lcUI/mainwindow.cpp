#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "windowmanager.h"

using namespace lc::ui;

MainWindow::MainWindow()
    :
    ui(new Ui::MainWindow),
    linePatternSelect(&cadMdiChild, this, true, true),
    lineWidthSelect(cadMdiChild.metaInfoManager(), this, true, true),
    colorSelect(cadMdiChild.metaInfoManager(), this, true, true),
    cliCommand(this),
    toolbar(&luaInterface, this)
{
    ui->setupUi(this);
    // new document and set mainwindow attributes
    cadMdiChild.newDocument();
    setWindowTitle(QObject::tr("LibreCAD"));
    setUnifiedTitleAndToolBarOnMac(true);
    setCentralWidget(&cadMdiChild);
    
    layers.setMdiChild(&cadMdiChild);

    // add widgets to correct positions
    addDockWidget(Qt::RightDockWidgetArea, &layers);
    addDockWidget(Qt::BottomDockWidgetArea, &cliCommand);
    addDockWidget(Qt::TopDockWidgetArea, &toolbar);

    toolbar.initializeToolbar(&linePatternSelect, &lineWidthSelect, &colorSelect);
    cadMdiChild.viewer()->autoScale();

    initMenuAPI();

    // connect required signals and slots
    ConnectInputEvents();

    // open qt bridge and run lua scripts
    luaInterface.initLua(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::runOperation(kaguya::LuaRef operation, const std::string& init_method){
    cliCommand.setFocus();
    luaInterface.finishOperation();
    kaguya::State state(luaInterface.luaState());

    // if current operation had extra operation toolbar icons, add them
    if (!operation["operation_options"].isNilref())
    {
        if (operation_options.find(operation["command_line"].get<std::string>() + init_method) != operation_options.end()) {
            std::vector<kaguya::LuaRef>& options = operation_options[operation["command_line"].get<std::string>() + init_method];

            for (auto op : options) {
                // run operation which adds option icon to toolbar
                op();
            }
        }else if (operation_options.find(operation["command_line"]) != operation_options.end()) {
            std::vector<kaguya::LuaRef>& options = operation_options[operation["command_line"]];

            for (auto op : options) {
                // run operation which adds option icon to toolbar
                op();
            }
        }
    }

    // add toolbar cancel button
    state.dostring("finish_op = function() finish_operation() end");
    toolbar.addButton("", ":/icons/quit.svg", "Current operation", state["finish_op"], "Cancel");
    state["finish_op"] = nullptr;

    // call operation to run CreateOperations init method etc
    luaInterface.setOperation(operation.call<kaguya::LuaRef>());
    kaguya::LuaRef op = luaInterface.operation();
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
    toolbar.removeGroupByName("Current operation");
    cadMdiChild.viewer()->setOperationActive(false);
}

lc::ui::widgets::CliCommand* MainWindow::getCliCommand(){
    return &cliCommand;
}

lc::ui::CadMdiChild* MainWindow::getCadMdiChild() {
    return &cadMdiChild;
}

lc::ui::widgets::Toolbar* MainWindow::getToolbar() {
    return &toolbar;
}

lc::ui::widgets::Layers* MainWindow::getLayers() {
    return &layers;
}

void MainWindow::ConnectInputEvents()
{   
    // CadMdiChild connections
    QObject::connect(cadMdiChild.viewerProxy(), &LCADViewerProxy::mousePressEvent, this, &MainWindow::triggerMousePressed);
    QObject::connect(cadMdiChild.viewerProxy(), &LCADViewerProxy::mouseReleaseEvent, this, &MainWindow::triggerMouseReleased);
    QObject::connect(cadMdiChild.viewerProxy(), &LCADViewerProxy::mouseMoveEvent, this, &MainWindow::triggerMouseMoved);
    QObject::connect(cadMdiChild.viewerProxy(), &LCADViewerProxy::keyPressEvent, this, &MainWindow::triggerKeyPressed);
    QObject::connect(&cadMdiChild, &CadMdiChild::keyPressed, &cliCommand, &widgets::CliCommand::onKeyPressed);

    // CliCommand connections
    QObject::connect(&cliCommand, &widgets::CliCommand::coordinateEntered, this, &MainWindow::triggerCoordinateEntered);
    QObject::connect(&cliCommand, &widgets::CliCommand::relativeCoordinateEntered, this, &MainWindow::triggerRelativeCoordinateEntered);
    QObject::connect(&cliCommand, &widgets::CliCommand::numberEntered, this, &MainWindow::triggerNumberEntered);
    QObject::connect(&cliCommand, &widgets::CliCommand::textEntered, this, &MainWindow::triggerTextEntered);
    QObject::connect(&cliCommand, &widgets::CliCommand::finishOperation, this, &MainWindow::triggerFinishOperation);
    QObject::connect(&cliCommand, &widgets::CliCommand::commandEntered, this, &MainWindow::triggerCommandEntered);

    // Layers to select tools connections
    QObject::connect(&layers, &widgets::Layers::layerChanged, &linePatternSelect, &widgets::LinePatternSelect::onLayerChanged);
    QObject::connect(&layers, &widgets::Layers::layerChanged, &lineWidthSelect, &widgets::LineWidthSelect::onLayerChanged);
    QObject::connect(&layers, &widgets::Layers::layerChanged, &colorSelect, &widgets::ColorSelect::onLayerChanged);

    // Other
    QObject::connect(this, &MainWindow::point, this, &MainWindow::triggerPoint);
    QObject::connect(findMenuItemByObjectName("actionExit"), &QAction::triggered, this, &MainWindow::close);

    // File connections
    QObject::connect(findMenuItemByObjectName("actionNew"), &QAction::triggered, this, &MainWindow::newFile);
    QObject::connect(findMenuItemByObjectName("actionOpen"), &QAction::triggered, this, &MainWindow::openFile);
    QObject::connect(findMenuItemByObjectName("actionSave_2"), &QAction::triggered, &cadMdiChild, &CadMdiChild::saveFile);
    QObject::connect(findMenuItemByObjectName("actionSave_As"), &QAction::triggered, &cadMdiChild, &CadMdiChild::saveAsFile);

    // Edit connections
    QObject::connect(findMenuItemByObjectName("actionUndo"), &QAction::triggered, this, &MainWindow::undo);
    QObject::connect(findMenuItemByObjectName("actionRedo"), &QAction::triggered, this, &MainWindow::redo);
    QObject::connect(findMenuItemByObjectName("actionSelect_All"), &QAction::triggered, this, &MainWindow::selectAll);
    QObject::connect(findMenuItemByObjectName("actionSelect_None"), &QAction::triggered, this, &MainWindow::selectNone);
    QObject::connect(findMenuItemByObjectName("actionInvert_Selection"), &QAction::triggered, this, &MainWindow::invertSelection);
}

/* Menu functions */

void MainWindow::connectMenuItem(const std::string& itemName, kaguya::LuaRef callback)
{
    luaInterface.luaConnect(findMenuItemByObjectName(itemName.c_str()), "triggered(bool)", callback);
}

void MainWindow::initMenuAPI() {
    QList<QMenu*> allMenus = menuBar()->findChildren<QMenu*>(QString(), Qt::FindDirectChildrenOnly);

    for (QMenu* current_menu : allMenus)
    {
        api::Menu* menu = static_cast<api::Menu*>(current_menu);
        this->menuBar()->addAction(menu->menuAction());
        menuMap[menu->title()] = menu;
        menu->setLuaInterface(&luaInterface, false);

        QList<QMenu*> allMenusOfCurrentMenu = menu->findChildren<QMenu*>(QString(), Qt::FindDirectChildrenOnly);

        for (QMenu* currentChildMenu : allMenusOfCurrentMenu)
        {
            if (currentChildMenu != nullptr) {
                menu->addAction(currentChildMenu->menuAction());
            }
        }

        addActionsAsMenuItem(menu);
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

    for (QAction* act : menuItemsToBeAdded)
    {
        menu->addAction(act);
    }

    // reorder menu to appear below
    for (QAction* action : actions)
    {
        if (action->menu()) {
            menu->insertMenu(menuItemsToBeAdded.last(), action->menu());
        }
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
    menuBar()->addMenu(newMenu);
    newMenu->setLuaInterface(&luaInterface);

    menuMap[QString(menuName.c_str())] = newMenu;

    return newMenu;
}

void MainWindow::addMenu(lc::ui::api::Menu* menu) {
    if (checkForMenuOfSameLabel(menu->getLabel())) {
        return;
    }

    menuMap[menu->title()] = menu;
    menuBar()->addMenu(menu);
    menu->setLuaInterface(&luaInterface);
}

api::Menu* MainWindow::getMenu(const std::string& menuName) {
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

lc::ui::api::Menu* MainWindow::getMenu(int pos) {
    QList<QAction*> menuList = menuBar()->actions();

    if (pos < 0 || pos >= menuList.size()) {
        return nullptr;
    }

    return static_cast<lc::ui::api::Menu*>(menuList[pos]->menu());
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
    lc::ui::api::Menu* menuremove = getMenu(menuLabel);
    menuremove->remove();
}

void MainWindow::removeMenu(int position) {
    lc::ui::api::Menu* menuremove = getMenu(position);
    menuremove->remove();
}

/* Trigger slots */

void MainWindow::triggerMousePressed()
{
    lc::geo::Coordinate cursorPos = cadMdiChild.cursor()->position();
    kaguya::State state(luaInterface.luaState());
    state["mousePressed"] = kaguya::NewTable();
    state["mousePressed"]["position"] = cursorPos;
    state["mousePressed"]["widget"] = &cadMdiChild;
    luaInterface.triggerEvent("point", state["mousePressed"]);

    emit point(cursorPos);
}

void MainWindow::triggerMouseReleased()
{
    kaguya::State state(luaInterface.luaState());
    state["mouseRelease"] = kaguya::NewTable();
    state["mouseRelease"]["widget"] = &cadMdiChild;
    luaInterface.triggerEvent("selectionChanged", state["mouseRelease"]);
}

void MainWindow::triggerMouseMoved()
{
    lc::geo::Coordinate cursorPos = cadMdiChild.cursor()->position();
    kaguya::State state(luaInterface.luaState());
    state["mouseMove"] = kaguya::NewTable();
    state["mouseMove"]["position"] = cursorPos;
    state["mouseMove"]["widget"] = &cadMdiChild;
    luaInterface.triggerEvent("mouseMove", state["mouseMove"]);
}

void MainWindow::triggerKeyPressed(int key)
{
    if (key == Qt::Key_Escape)
    {
        // run finish operation
        auto state = luaInterface.luaState();
        luaInterface.triggerEvent("finishOperation", kaguya::LuaRef(state));
    }
    else
    {
        kaguya::State state(luaInterface.luaState());
        state["keyEvent"] = kaguya::NewTable();
        state["keyEvent"]["key"] = key;
        state["keyEvent"]["widget"] = &cadMdiChild;
        luaInterface.triggerEvent("keyPressed", state["keyEvent"]);
    }
}

void MainWindow::triggerCoordinateEntered(lc::geo::Coordinate coordinate)
{
    kaguya::State state(luaInterface.luaState());
    state["coordinateEntered"] = kaguya::NewTable();
    state["coordinateEntered"]["position"] = coordinate;
    state["coordinateEntered"]["widget"] = &cadMdiChild;
    luaInterface.triggerEvent("point", state["coordinateEntered"]);

    emit point(coordinate);
}

void MainWindow::triggerRelativeCoordinateEntered(lc::geo::Coordinate coordinate)
{
    kaguya::State state(luaInterface.luaState());
    state["relCoordinateEntered"] = kaguya::NewTable();
    state["relCoordinateEntered"]["position"] = lastPoint + coordinate;
    state["relCoordinateEntered"]["widget"] = &cadMdiChild;
    luaInterface.triggerEvent("point", state["relCoordinateEntered"]);

    emit point(lastPoint + coordinate);
}

void MainWindow::triggerNumberEntered(double number)
{
    kaguya::State state(luaInterface.luaState());
    state["numberEntered"] = kaguya::NewTable();
    state["numberEntered"]["number"] = number;
    state["numberEntered"]["widget"] = &cadMdiChild;
    luaInterface.triggerEvent("number", state["numberEntered"]);
}

void MainWindow::triggerTextEntered(QString text)
{
    kaguya::State state(luaInterface.luaState());
    state["textEntered"] = kaguya::NewTable();
    state["textEntered"]["text"] = text;
    state["textEntered"]["widget"] = &cadMdiChild;
    luaInterface.triggerEvent("text", state["textEntered"]);
}

void MainWindow::triggerFinishOperation()
{
    auto state = luaInterface.luaState();
    luaInterface.triggerEvent("operationFinished", kaguya::LuaRef(state));
    luaInterface.triggerEvent("finishOperation", kaguya::LuaRef(state));
}

void MainWindow::triggerCommandEntered(QString command)
{
    cliCommand.runCommand(command.toStdString().c_str());
}

void MainWindow::triggerPoint(lc::geo::Coordinate coordinate)
{
    lastPoint = coordinate;
    cadMdiChild.viewer()->docCanvas()->selectPoint(coordinate.x(), coordinate.y());
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
    cadMdiChild.undoManager()->undo();
}

void MainWindow::redo()
{
    cadMdiChild.undoManager()->redo();
}

void MainWindow::selectAll()
{
    cadMdiChild.viewer()->docCanvas()->selectAll();
}

void MainWindow::selectNone()
{
    cadMdiChild.viewer()->docCanvas()->removeSelection();
}

void MainWindow::invertSelection()
{
    cadMdiChild.viewer()->docCanvas()->inverseSelection();
}
