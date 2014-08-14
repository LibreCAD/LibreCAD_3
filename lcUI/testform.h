#pragma once

#include <QWidget>

namespace Ui {
    class testForm;
}

class testForm : public QWidget {
        Q_OBJECT

    public:
        explicit testForm(QWidget* parent = 0);
        ~testForm();

    private:
        Ui::testForm* ui;
};
