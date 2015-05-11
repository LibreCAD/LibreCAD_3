#pragma once

#include <QWidget>

#include "../imainwindow.h"

namespace Ui {
    class LCMainToolbar;
}

class LCMainToolbar : public QWidget {
Q_OBJECT

public:
    explicit LCMainToolbar(IMainWindow *parent = 0);

    ~LCMainToolbar();

private slots:

    void on_toolButton_clicked();

    void on_toolButton_2_clicked();

    void on_toolButton_3_clicked();

    void on_tb_builderTest_clicked();


    void on_tb_snapGrid_clicked();

    void on_tb_snapEntity_clicked();
    void on_tb_snapEntityPath_clicked();

    void on_tb_snapLogical_clicked();
    void on_tb_snapIntersections_clicked();

private:
    Ui::LCMainToolbar *ui;
    IMainWindow *_parent;
};

// LCMAINTOOLBAR_H
