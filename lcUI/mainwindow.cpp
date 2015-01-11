
#include <memory>

#include <QTime>

#include "mainwindow.h"
#include "ui_mainwindow.h"


#include "ui/lcmaintoolbar.h"
#include "ui/clicommand.h"

#include <QtGui>
#include <QMdiSubWindow>
#include <cmath>
#include <QFileDialog>
#include <ui/luascript.h>


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
    installEventFilter(this);

    on_actionNew_triggered();

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionNew_triggered() {
    CadMdiChild* child = createMdiChild();
    child->newDocument();
    ui->mdiArea->addSubWindow(child);
    child->showMaximized();
}

void MainWindow::on_actionOpen_triggered() {

    // open file dialog box
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"),
                                                    QString(),
                                                    tr("dxf(*.dxf);;dwg(*.dwg)"));

    if (!filename.isEmpty()) {
        CadMdiChild* child = createMdiChild();
        child->import(filename.toStdString());
        ui->mdiArea->addSubWindow(child);
        child->showMaximized();
    } else {
        // DOn't do a thing
    }

}

CadMdiChild* MainWindow::createMdiChild() {
    CadMdiChild* child = new CadMdiChild;
    return child;
}

CadMdiChild* MainWindow::activeMdiChild() const {
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

    // Left docing are + toolbar
    QDockWidget* dockWidget = new QDockWidget("", this);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea);
    dockWidget->setMaximumWidth(33);
    dockWidget->setWidget(new LCMainToolbar(this));
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

    // Add Cli COmmand interpeter
    //    QDockWidget* _cliCommand = new CliCommand(this);
    //    addDockWidget(Qt::BottomDockWidgetArea, _cliCommand);

    // Add Lua Script
    QDockWidget* _luaScript = new LuaScript(this);
    addDockWidget(Qt::RightDockWidgetArea, _luaScript);

}


bool MainWindow::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Escape) {
            activeMdiChild()->cancelCurrentOperations();
            // Special tab handling
            return true;
        } else {
            return false;
        }
    }

    return false;
}
