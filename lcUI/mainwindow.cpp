#include "mainwindow.h"

#include "luainterface.h"

using namespace lc::ui::widgets;

MainWindow::MainWindow()
    :
    linePatternSelect(&cadMdiChild, this, true, true),
    lineWidthSelect(cadMdiChild.metaInfoManager(), this, true, true),
    colorSelect(cadMdiChild.metaInfoManager(), this, true, true)
{
    cadMdiChild.newDocument();
    setWindowTitle(QObject::tr("LibreCAD"));
    setUnifiedTitleAndToolBarOnMac(true);
    setCentralWidget(&cadMdiChild);
    
    layers.setMdiChild(&cadMdiChild);
    addDockWidget(Qt::RightDockWidgetArea, &layers);

    show();
}
