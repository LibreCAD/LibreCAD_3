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

void RadioButtonGUI::addCallback(lc::scripting::ScriptCallback cb) {
    _callbacks.push_back(std::move(cb));
}

void RadioButtonGUI::callbackCalled(bool toggled) {
    // Phase 4 PR-5a — neutral callback invocation.
    for (auto& cb : _callbacks) {
        cb.call(toggled);
    }
}

void RadioButtonGUI::getValue(lc::scripting::Map& map) {
    (*map)[_key] = lc::scripting::ScriptValue(isChecked());
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
