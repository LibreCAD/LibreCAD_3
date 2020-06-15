#include "dialogwidget.h"
#include "ui_dialogwidget.h"

#include "buttongroupgui.h"

#include <iostream>
#include <QGridLayout>

using namespace lc::ui::api;

DialogWidget::DialogWidget(const std::string& dialogTitle, QWidget* parent)
    : 
    QDialog(parent),
    ui(new Ui::DialogWidget)
{
    ui->setupUi(this);
    this->setWindowTitle(QString(dialogTitle.c_str()));
    show();

    auto gridLayout = qobject_cast<QGridLayout*>(this->layout());
    vboxlayout = gridLayout->findChild<QVBoxLayout*>("verticalLayout");
}

DialogWidget::~DialogWidget()
{
    delete ui;
}

void DialogWidget::setTitle(const std::string& newTitle) {
    this->setWindowTitle(QString(newTitle.c_str()));
}

std::string DialogWidget::title() const {
    return this->windowTitle().toStdString();
}

void DialogWidget::addWidget(InputGUI* guiWidget) {
    guiWidget->setParent(this);
    vboxlayout->addWidget(guiWidget);
    _inputWidgets.push_back(guiWidget);
}

void DialogWidget::addWidget(ButtonGUI* buttonWidget) {
    ButtonGroupGUI* buttonGroup = new ButtonGroupGUI(buttonWidget->label() + "_group");
    buttonGroup->addButton(buttonWidget);
    addWidget(buttonGroup);
}

const std::vector<InputGUI*>& DialogWidget::inputWidgets() {
    return _inputWidgets;
}
