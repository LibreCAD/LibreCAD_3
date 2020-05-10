#include "mainwindow.h"

#include "luainterface.h"

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
    CreateMenu();

    showMaximized();
}

void MainWindow::CreateMenu()
{
    QMenuBar* menu = this->menuBar();

    QMenu* drawMenu = menu->addMenu(QString("Draw"));
    auto lineAction = drawMenu->addAction(QString("Line"));
}
