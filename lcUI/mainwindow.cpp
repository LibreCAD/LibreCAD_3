#include "mainwindow.h"

#include "luainterface.h"

using namespace lc::ui::widgets;

MainWindow::MainWindow()
    :
    //linePatternSelect(&cadMdiChild, this, true, true),
    //lineWidthSelect(cadMdiChild.metaInfoManager(), this, true, true),
    //colorSelect(cadMdiChild.metaInfoManager(), this, true, true),
    cliCommand(this),
    toolbar(this)
{
    cadMdiChild.newDocument();
    setWindowTitle(QObject::tr("LibreCAD"));
    setUnifiedTitleAndToolBarOnMac(true);
    setCentralWidget(&cadMdiChild);
    
    layers.setMdiChild(&cadMdiChild);

    addDockWidget(Qt::RightDockWidgetArea, &layers);
    addDockWidget(Qt::BottomDockWidgetArea, &cliCommand);
    addDockWidget(Qt::TopDockWidgetArea, &toolbar);

    showMaximized();
}
