#include "buttongui.h"

#include <QVBoxLayout>

using namespace lc::ui::api;

ButtonGUI::ButtonGUI(std::string label, QWidget* parent)
    :
    InputGUI(label, parent)
{
    _type = "button";
    _pushButton = new QPushButton(QString(label.c_str()));
    this->setLayout(new QVBoxLayout());
    this->layout()->addWidget(_pushButton);
    connect(_pushButton, &QPushButton::clicked, this, &ButtonGUI::callbackCalled);
}

void ButtonGUI::setLabel(const std::string& newLabel) {
    InputGUI::setLabel(newLabel);
    _pushButton->setText(QString(newLabel.c_str()));
}

void ButtonGUI::addCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

void ButtonGUI::callbackCalled() {
    for (kaguya::LuaRef& cb : _callbacks){
        cb();
    }
}

void ButtonGUI::getLuaValue(kaguya::LuaRef& table) {
}

void ButtonGUI::click() {
    _pushButton->click();
}

void ButtonGUI::copyValue(QDataStream& stream) {
}

void ButtonGUI::pasteValue(QDataStream& stream) {
}
