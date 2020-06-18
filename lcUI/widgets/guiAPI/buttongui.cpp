#include "buttongui.h"

using namespace lc::ui::api;

ButtonGUI::ButtonGUI(std::string label, QWidget* parent)
    :
    QPushButton(QString(label.c_str()), parent),
    _label(label)
{
    connect(this, &ButtonGUI::clicked, this, &ButtonGUI::callbackCalled);
}

std::string ButtonGUI::label() const {
    return _label;
}

void ButtonGUI::setLabel(const std::string& newLabel) {
    _label = newLabel;
    this->setText(QString(newLabel.c_str()));
}

void ButtonGUI::addCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

void ButtonGUI::callbackCalled() {
    for (kaguya::LuaRef& cb : _callbacks){
        cb();
    }
}

void ButtonGUI::setKey(const std::string& keyIn) {
    _key = keyIn;
}
