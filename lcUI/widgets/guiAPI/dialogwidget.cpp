#include "dialogwidget.h"
#include "ui_dialogwidget.h"

#include "horizontalgroupgui.h"
#include "radiogroupgui.h"
#include "coordinategui.h"
#include "entitygui.h"

#include <iostream>
#include <QGridLayout>

using namespace lc::ui::api;

DialogWidget::DialogWidget(const std::string& dialogTitle, lc::ui::MainWindow* mainWindow, QWidget* parent)
    : 
    QDialog(parent),
    InputGUIContainer(dialogTitle, mainWindow),
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
    InputGUIContainer::setLabel(newTitle);
}

std::string DialogWidget::title() const {
    return this->windowTitle().toStdString();
}

bool DialogWidget::addWidget(const std::string& key, InputGUI* guiWidget) {
    bool success = InputGUIContainer::addWidget(key, guiWidget);
    guiWidget->setParent(this);
    vboxlayout->addWidget(guiWidget);
    return success;
}

bool DialogWidget::addWidget(const std::string& key, ButtonGUI* buttonWidget) {
    return InputGUIContainer::addWidget(key, buttonWidget);
}

bool DialogWidget::addWidget(const std::string& key, CheckBoxGUI* checkboxWidget) {
    return InputGUIContainer::addWidget(key, checkboxWidget);
}

void DialogWidget::setFinishButton(ButtonGUI* buttonWidget) {
    if (buttonWidget != nullptr) {
        connect(buttonWidget, &ButtonGUI::clicked, this, &DialogWidget::finishCallbacks);
    }
}

void DialogWidget::finishCallbacks() {
    for (kaguya::LuaRef& cb : _callbacks) {
        kaguya::LuaRef result = generateInfo(cb.state());
        cb(result);
    }
}
