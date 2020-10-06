#include "checkboxgui.h"

#include <QHBoxLayout>

using namespace lc::ui::api;

CheckBoxGUI::CheckBoxGUI(std::string label, bool checked, QWidget* parent)
    :
    InputGUI(label, parent)
{
    _type = "checkbox";
    _checkBox = new QCheckBox(QString(label.c_str()));
    this->setLayout(new QHBoxLayout());
    this->layout()->addWidget(_checkBox);
    _checkBox->setChecked(checked);

    qobject_cast<QHBoxLayout*>(this->layout())->insertStretch(0);
    connect(_checkBox, &QCheckBox::stateChanged, this, &CheckBoxGUI::callbackCalled);
}

void CheckBoxGUI::setLabel(const std::string& newLabel) {
    InputGUI::setLabel(newLabel);
    _checkBox->setText(QString(newLabel.c_str()));
}

void CheckBoxGUI::addCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

void CheckBoxGUI::callbackCalled(int state) {
    for (kaguya::LuaRef& cb : _callbacks) {
        cb(state == Qt::Checked);
    }
}

void CheckBoxGUI::getLuaValue(kaguya::LuaRef& table) {
    table[_key] = _checkBox->isChecked();
}

bool CheckBoxGUI::value() const {
    return _checkBox->isChecked();
}

void CheckBoxGUI::setValue(bool check) {
    _checkBox->setChecked(check);
}

void CheckBoxGUI::copyValue(QDataStream& stream) {
    stream << value();
}

void CheckBoxGUI::pasteValue(QDataStream& stream) {
    bool val;
    stream >> val;
    setValue(val);
    callbackCalled(_checkBox->isChecked());
}

void CheckBoxGUI::hideLabel() {
    _checkBox->setText("");
}
