#include "dialogwidget.h"
#include "ui_dialogwidget.h"

#include "horizontalgroupgui.h"

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
    if (guiWidget == nullptr) {
        return;
    }

    guiWidget->setParent(this);
    vboxlayout->addWidget(guiWidget);
    _inputWidgets.push_back(guiWidget);
}

void DialogWidget::addWidget(ButtonGUI* buttonWidget) {
    if (buttonWidget == nullptr) {
        return;
    }

    HorizontalGroupGUI* horizGroup = new HorizontalGroupGUI(buttonWidget->label() + "_group");
    horizGroup->addWidget(buttonWidget);
    addWidget(horizGroup);
}

void DialogWidget::addWidget(CheckBoxGUI* checkboxWidget) {
    if (checkboxWidget == nullptr) {
        return;
    }

    HorizontalGroupGUI* horizGroup = new HorizontalGroupGUI(checkboxWidget->label() + "_group");
    horizGroup->addWidget(checkboxWidget);
    addWidget(horizGroup);
}

const std::vector<InputGUI*>& DialogWidget::inputWidgets() {
    return _inputWidgets;
}

void DialogWidget::setFinishButton(ButtonGUI* buttonWidget) {
    if (buttonWidget != nullptr) {
        connect(buttonWidget, &ButtonGUI::clicked, this, &DialogWidget::finishCallbacks);
    }
}

void DialogWidget::finishCallbacks() {
    for (kaguya::LuaRef& cb : _callbacks) {
        kaguya::LuaRef result = generateDialogInfo(cb.state());
        cb(result);
    }
}

void DialogWidget::addFinishCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

kaguya::LuaRef DialogWidget::generateDialogInfo(kaguya::State state) {
    std::string refName = title() + "_table";
    state[refName] = kaguya::NewTable();
    kaguya::LuaRef table = state[refName];
    std::vector<InputGUI*> inputGUIs = inputWidgets();

    for (InputGUI* inputgui : inputGUIs) {
        inputgui->getLuaValue(table);
    }

    return table;
}
