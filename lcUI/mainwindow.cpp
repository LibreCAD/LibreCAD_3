#include "mainwindow.h"

#include<QMenuBar>
#include<QMenu>

using namespace lc::ui::widgets;

MainWindow::MainWindow()
    :
    linePatternSelect(&cadMdiChild, this, true, true),
    lineWidthSelect(cadMdiChild.metaInfoManager(), this, true, true),
    colorSelect(cadMdiChild.metaInfoManager(), this, true, true),
    cliCommand(this),
    toolbar(this)
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
