#ifndef TESTFORM_H
#define TESTFORM_H

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

#endif // TESTFORM_H
