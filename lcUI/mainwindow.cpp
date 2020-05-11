#include "mainwindow.h"

#include<QMenuBar>
#include<QMenu>

using namespace lc::ui;

MainWindow::MainWindow(lc::ui::LuaInterface* luaInterface)
    :
    linePatternSelect(&cadMdiChild, this, true, true),
    lineWidthSelect(cadMdiChild.metaInfoManager(), this, true, true),
    colorSelect(cadMdiChild.metaInfoManager(), this, true, true),
    cliCommand(this),
    toolbar(this),
    luaInterface(luaInterface)
{
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

    toolbar.InitializeToolbar(&linePatternSelect, &lineWidthSelect, &colorSelect);
    ConnectInputEvents();
    //showMaximized();
}

lc::ui::widgets::CliCommand* MainWindow::getCliCommand(){
    return &cliCommand;
}

lc::ui::CadMdiChild* MainWindow::getCadMdiChild() {
    return &cadMdiChild;
}

QAction* MainWindow::createMenu()
{
    QMenuBar* menu = this->menuBar();

    QMenu* drawMenu = menu->addMenu(QString("Draw"));
    QAction* lineAction = drawMenu->addAction(QString("Line"));

    showMaximized();
    return lineAction;
}

void MainWindow::ConnectInputEvents()
{
    cadMdiChild.viewer()->autoScale();
    
    luaInterface->qtConnect(cadMdiChild.viewerProxy(), "mousePressEvent()", this, "triggerMousePressed()");
    luaInterface->qtConnect(cadMdiChild.viewerProxy(), "mouseReleaseEvent()", this, "triggerMouseReleased()");
    luaInterface->qtConnect(cadMdiChild.viewerProxy(), "mouseMoveEvent()", this, "triggerMouseMoved()");
    luaInterface->qtConnect(cadMdiChild.viewerProxy(), "keyPressEvent(int)", this, "triggerKeyPressed(int)");
    luaInterface->qtConnect(&cadMdiChild, "keyPressed(QKeyEvent*)", &cliCommand, "onKeyPressed(QKeyEvent*)");
}

void MainWindow::triggerMousePressed()
{
    lc::geo::Coordinate cursorPos = cadMdiChild.cursor()->position();
    auto state = luaInterface->luaState();
    kaguya::LuaTable data(state);
    data["position"] = cursorPos;
    data["widget"] = &cadMdiChild;
    luaInterface->triggerEvent("point", data);
}

void MainWindow::triggerMouseReleased()
{
    auto state = luaInterface->luaState();
    kaguya::LuaTable data(state);
    data["widget"] = &cadMdiChild;
    luaInterface->triggerEvent("selectionChanged", data);
}

void MainWindow::triggerMouseMoved()
{
    lc::geo::Coordinate cursorPos = cadMdiChild.cursor()->position();
    auto state = luaInterface->luaState();
    kaguya::LuaTable data(state);
    data["position"] = cursorPos;
    data["widget"] = &cadMdiChild;
    luaInterface->triggerEvent("mouseMove", data);
}

void MainWindow::triggerKeyPressed(int key)
{
    if (key == Qt::Key_Escape)
    {
        // run finish operation
    }
    else
    {
        auto state = luaInterface->luaState();
        kaguya::LuaTable data(state);
        data["key"] = key;
        data["widget"] = &cadMdiChild;
        luaInterface->triggerEvent("keyPressed", data);
    }
}
