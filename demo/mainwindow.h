#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "cad/document/document.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = 0);
        virtual ~MainWindow();

    private:
        Ui::MainWindow* ui;
        lc::Document* _document;
};

#endif // MAINWINDOW_H
