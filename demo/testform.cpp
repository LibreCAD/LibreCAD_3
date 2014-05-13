#include "testform.h"
#include "ui_testform.h"

testForm::testForm(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::testForm) {
    ui->setupUi(this);
}

testForm::~testForm() {
    delete ui;
}
