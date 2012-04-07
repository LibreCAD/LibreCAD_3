#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

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

    protected:
        void wheelEvent(QWheelEvent* event);
    private slots:
        void on_addEntities_clicked();

        void on_addCircles_clicked();

        void on_undoButton_clicked();

        void on_redoButtom_clicked();

        void on_clearUndoables_clicked();

        void on_addArcs_clicked();

        void on_addEllipse_clicked();

    private:
        Ui::MainWindow* ui;
        lc::AbstractDocument* _document;
        lc::UndoManager* _undoManager;

};

#endif // MAINWINDOW_H
