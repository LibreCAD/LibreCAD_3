#include "linepatternpathpart.h"
#include "ui_linepatternpathpart.h"

#include <iostream>

LinePatternPathPart::LinePatternPathPart(QWidget* parent) :
    LinePatternPathPart(1, parent) {
}

LinePatternPathPart::LinePatternPathPart(double value, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LinePatternPathPart) {

    ui->setupUi(this);

    ui->type->addItem(PATH_PLAIN_NAME);
    ui->type->addItem(PATH_DOT_NAME);
    ui->type->addItem(PATH_SPACE_NAME);

    ui->value->setValue(std::fabs(value));

    int typeIndex;
    if(value == 0) {
        typeIndex = ui->type->findText(PATH_DOT_NAME);
        ui->value->setDisabled(true);
    }
    else if(value > 0) {
        typeIndex = ui->type->findText(PATH_PLAIN_NAME);
    }
    else {
        typeIndex = ui->type->findText(PATH_SPACE_NAME);
    }


    if (typeIndex != -1) {
        ui->type->setCurrentIndex(typeIndex);
    }

    connect(ui->type, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onTypeChanged(const QString&)));
    connect(ui->value, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
}

LinePatternPathPart::PathType LinePatternPathPart::type() {
    if(ui->type->currentText() == PATH_PLAIN_NAME) {
        return PATH_PLAIN;
    }
    else if(ui->type->currentText() == PATH_DOT_NAME) {
        return PATH_DOT;
    }
    else if(ui->type->currentText() == PATH_SPACE_NAME) {
        return PATH_SPACE;
    }

    return PATH_PLAIN;
}

double LinePatternPathPart::value() {
    return ui->value->value();
}

void LinePatternPathPart::onTypeChanged(const QString &value) {
    if(ui->type->currentText() == PATH_DOT_NAME) {
        ui->value->setValue(0);
        ui->value->setDisabled(true);
    }
    else {
        ui->value->setValue(1);
        ui->value->setDisabled(false);
    }

    emit update();
}

void LinePatternPathPart::onValueChanged(double) {
    emit update();
}