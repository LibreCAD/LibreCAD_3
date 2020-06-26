#include "radiobuttongui.h"

using namespace lc::ui::api;

RadioButtonGUI::RadioButtonGUI(std::string label, QWidget* parent)
    :
    QRadioButton(QString(label.c_str()), parent),
    _label(label)
{
    connect(this, &RadioButtonGUI::toggled, this, &RadioButtonGUI::callbackCalled);
}

std::string RadioButtonGUI::label() const {
    return _label;
}

void RadioButtonGUI::setLabel(const std::string& newLabel) {
    _label = newLabel;
    this->setText(QString(newLabel.c_str()));
}

void RadioButtonGUI::addCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

void RadioButtonGUI::callbackCalled(bool toggled) {
    for (kaguya::LuaRef& cb : _callbacks) {
        cb(toggled);
    }
}

void RadioButtonGUI::getLuaValue(kaguya::LuaRef& table) {
    table[_key] = isChecked();
}

void RadioButtonGUI::setKey(const std::string& keyIn) {
    _key = keyIn;
}

bool RadioButtonGUI::value() const {
    return this->isChecked();
}

void RadioButtonGUI::setValue(bool toggle) {
    this->setChecked(toggle);
}
