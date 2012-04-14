#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

#include "cadmdichild.h"

#include "cad/document/abstractdocument.h"
#include "cad/document/undomanager.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = 0);
        virtual ~MainWindow();

        int randInt(int low, int high);

        CadMdiChild* createMdiChild();
        CadMdiChild* activeMdiChild();
        void updateMenus();
        void setActiveSubWindow(QWidget* window);

    protected:
    private slots:



        void on_actionNew_triggered();
        void on_actionRedo_triggered();
        void on_actionUndo_triggered();
        void on_actionAdd_Random_Circles_triggered();
        void on_actionAdd_Random_Arc_triggered();
        void on_actionClear_Undoable_Stack_triggered();
        void on_actionAdd_Random_Lines_triggered();

    private:
        Ui::MainWindow* ui;
        QSignalMapper* windowMapper;

};

#endif // MAINWINDOW_H
