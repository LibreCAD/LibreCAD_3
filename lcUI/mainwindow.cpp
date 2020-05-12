#include "mainwindow.h"

#include<QMenuBar>
#include<QMenu>

#include <iostream>

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

    luaInterface->qtConnect(&cliCommand, "coordinateEntered(lc::geo::Coordinate)", this, "triggerCoordinateEntered(lc::geo::Coordinate)");
    luaInterface->qtConnect(&cliCommand, "numberEntered(double)", this, "triggerNumberEntered(double)");
    luaInterface->qtConnect(&cliCommand, "textEntered(QString)", this, "triggerTextEntered(QString)");
    luaInterface->qtConnect(&cliCommand, "finishOperation()", this, "triggerFinishOperation()");
    luaInterface->qtConnect(&cliCommand, "commandEntered(QString)", this, "triggerCommandEntered(QString)");
}

/* Trigger slots */

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

void MainWindow::triggerCoordinateEntered(lc::geo::Coordinate coordinate)
{
    auto state = luaInterface->luaState();
    kaguya::LuaTable data(state);
    data["position"] = coordinate;
    data["widget"] = &cadMdiChild;
    luaInterface->triggerEvent("point", data);
}

void MainWindow::triggerNumberEntered(double number)
{
    auto state = luaInterface->luaState();
    kaguya::LuaTable data(state);
    data["number"] = number;
    data["widget"] = &cadMdiChild;
    luaInterface->triggerEvent("number", data);
}

void MainWindow::triggerTextEntered(QString text)
{
    auto state = luaInterface->luaState();
    kaguya::LuaTable data(state);
    data["text"] = text;
    data["widget"] = &cadMdiChild;
    luaInterface->triggerEvent("text", data);
}

void MainWindow::triggerFinishOperation()
{
    auto state = luaInterface->luaState();
    kaguya::State s(state);
    s["id"] = 0;
    luaInterface->triggerEvent("operationFinished", s["id"]);
    luaInterface->triggerEvent("finishOperation", s["id"]);
}

void MainWindow::triggerCommandEntered(QString command)
{
    cliCommand.runCommand(command, cadMdiChild.id());
}
