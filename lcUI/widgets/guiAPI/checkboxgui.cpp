#include "checkboxgui.h"

using namespace lc::ui::api;

CheckBoxGUI::CheckBoxGUI(std::string label, bool checked, QWidget* parent)
    :
    QCheckBox(QString(label.c_str()), parent),
    _label(label)
{
    this->setChecked(checked);
    connect(this, &CheckBoxGUI::stateChanged, this, &CheckBoxGUI::callbackCalled);
}

std::string CheckBoxGUI::label() const {
    return _label;
}

void CheckBoxGUI::setLabel(const std::string& newLabel) {
    _label = newLabel;
    this->setText(QString(newLabel.c_str()));
}

void CheckBoxGUI::addCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

void CheckBoxGUI::callbackCalled(int state) {
    for (kaguya::LuaRef& cb : _callbacks) {
        cb(state == Qt::Checked);
    }
}
