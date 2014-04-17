#ifndef LCMAINTOOLBAR_H
#define LCMAINTOOLBAR_H

#include <QWidget>

#include "../imainwindow.h"

namespace Ui {
    class LCMainToolbar;
}

class LCMainToolbar : public QWidget {
        Q_OBJECT

    public:
        explicit LCMainToolbar(IMainWindow* parent = 0);
        ~LCMainToolbar();

    private slots:
        void on_toolButton_clicked();

        void on_toolButton_2_clicked();

        void on_toolButton_3_clicked();

        void on_toolButton_4_clicked();

        void on_toolButton_5_clicked();

private:
        Ui::LCMainToolbar* ui;
        IMainWindow* _parent;
};

#endif // LCMAINTOOLBAR_H
