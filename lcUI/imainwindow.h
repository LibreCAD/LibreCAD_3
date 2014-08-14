#pragma once

#include <QMainWindow>

#include "operations/operationmanager.h"
#include "cadmdichild.h"

class IMainWindow : public QMainWindow {
        Q_OBJECT
    public:
        IMainWindow(QWidget* parent) : QMainWindow(parent) {
        }

        virtual CadMdiChild* activeMdiChild() const = 0;
};
