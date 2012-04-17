
#include <memory>

#include <QTime>

#include "mainwindow.h"
#include "ui_mainwindow.h"


#include "ui/lcmaintoolbar.h"

#include <QtGui>

#include <math.h>


MainWindow::MainWindow(QWidget* parent) :
    IMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);


    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(ui->mdiArea);

    connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(updateMenus()));
    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)),
            this, SLOT(setActiveSubWindow(QWidget*)));

    setWindowTitle(tr("LibreCAD "));
    setUnifiedTitleAndToolBarOnMac(true);

    addToolbars();

    on_actionNew_triggered();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionNew_triggered() {
    CadMdiChild* child = createMdiChild();
    child->newDocument();
    child->showMaximized();
}

CadMdiChild* MainWindow::createMdiChild() {
    CadMdiChild* child = new CadMdiChild;
    ui->mdiArea->addSubWindow(child);
    return child;
}

CadMdiChild* MainWindow::activeMdiChild() {
    if (QMdiSubWindow* activeSubWindow = ui->mdiArea->activeSubWindow()) {
        return qobject_cast<CadMdiChild*>(activeSubWindow->widget());
    }

    return 0;
}
void MainWindow::setActiveSubWindow(QWidget* window) {
    if (!window) {
        return;
    }

    ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

void MainWindow::updateMenus() {
    bool hasMdiChild = (activeMdiChild() != 0);
    ui->actionUndo->setEnabled(hasMdiChild);
    ui->actionRedo->setEnabled(hasMdiChild);
}



void MainWindow::on_actionRedo_triggered() {
    if (activeMdiChild()) {
        activeMdiChild()->redo();
    }
}

void MainWindow::on_actionUndo_triggered() {
    if (activeMdiChild()) {
        activeMdiChild()->undo();
    }
}

void MainWindow::on_actionAdd_Random_Circles_triggered() {
    if (activeMdiChild()) {
        activeMdiChild()->on_addCircles_clicked();
    }
}

void MainWindow::on_actionAdd_Random_Arc_triggered() {
    if (activeMdiChild()) {
        activeMdiChild()->on_addArcs_clicked();
    }
}

void MainWindow::on_actionClear_Undoable_Stack_triggered() {
    if (activeMdiChild()) {
        activeMdiChild()->on_clearUndoables_clicked();
    }
}

void MainWindow::on_actionAdd_Random_Lines_triggered() {
    if (activeMdiChild()) {
        activeMdiChild()->on_actionAdd_Random_Lines_triggered();
    }
}

void MainWindow::addToolbars() {

    QDockWidget* dockWidget = new QDockWidget("", this);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea);
    dockWidget->setMaximumWidth(33);
    dockWidget->setWidget(new LCMainToolbar(this));
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);


}
