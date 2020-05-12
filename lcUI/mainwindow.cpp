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
    cadMdiChild.viewer()->autoScale();

    ConnectInputEvents();
    showMaximized();
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

    return lineAction;
}

void MainWindow::ConnectInputEvents()
{   
    QObject::connect(cadMdiChild.viewerProxy(), &LCADViewerProxy::mousePressEvent, this, &MainWindow::triggerMousePressed);
    QObject::connect(cadMdiChild.viewerProxy(), &LCADViewerProxy::mouseReleaseEvent, this, &MainWindow::triggerMouseReleased);
    QObject::connect(cadMdiChild.viewerProxy(), &LCADViewerProxy::mouseMoveEvent, this, &MainWindow::triggerMouseMoved);
    QObject::connect(cadMdiChild.viewerProxy(), &LCADViewerProxy::keyPressEvent, this, &MainWindow::triggerKeyPressed);
    QObject::connect(&cadMdiChild, &CadMdiChild::keyPressed, &cliCommand, &widgets::CliCommand::onKeyPressed);

    QObject::connect(&cliCommand, &widgets::CliCommand::coordinateEntered, this, &MainWindow::triggerCoordinateEntered);
    QObject::connect(&cliCommand, &widgets::CliCommand::relativeCoordinateEntered, this, &MainWindow::triggerRelativeCoordinateEntered);
    QObject::connect(&cliCommand, &widgets::CliCommand::numberEntered, this, &MainWindow::triggerNumberEntered);
    QObject::connect(&cliCommand, &widgets::CliCommand::textEntered, this, &MainWindow::triggerTextEntered);
    QObject::connect(&cliCommand, &widgets::CliCommand::finishOperation, this, &MainWindow::triggerFinishOperation);
    QObject::connect(&cliCommand, &widgets::CliCommand::commandEntered, this, &MainWindow::triggerCommandEntered);

    QObject::connect(this, &MainWindow::point, this, &MainWindow::triggerPoint);
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

    emit point(cursorPos);
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
        auto state = luaInterface->luaState();
        kaguya::State s(state);
        s["id"] = 0;
        luaInterface->triggerEvent("finishOperation", s["id"]);
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

    emit point(coordinate);
}

void MainWindow::triggerRelativeCoordinateEntered(lc::geo::Coordinate coordinate)
{
    auto state = luaInterface->luaState();
    kaguya::LuaTable data(state);
    data["position"] = lastPoint + coordinate;
    data["widget"] = &cadMdiChild;
    luaInterface->triggerEvent("point", data);

    emit point(lastPoint + coordinate);
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

void MainWindow::triggerPoint(lc::geo::Coordinate coordinate)
{
    lastPoint = coordinate;

    cadMdiChild.viewer()->docCanvas()->selectPoint(coordinate.x(), coordinate.y());
}
