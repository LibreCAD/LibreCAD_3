#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

#include "cad/document/abstractdocument.h"


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
    private:
        Ui::MainWindow* ui;
        lc::AbstractDocument* _document;
};

#endif // MAINWINDOW_H
