#ifndef IMAINWINDOW_H
#define IMAINWINDOW_H

#include <QMainWindow>

#include "cadmdichild.h"


class IMainWindow : public QMainWindow {
        Q_OBJECT
    public:
        IMainWindow(QWidget* parent) : QMainWindow(parent) {
        };

        virtual CadMdiChild* activeMdiChild() = 0;
};

#endif // IMAINWINDOW_H
