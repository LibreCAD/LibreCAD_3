#include "radiogroupgui.h"

using namespace lc::ui::api;

RadioGroupGUI::RadioGroupGUI(std::string label, QWidget* parent)
    :
    InputGUI(label, parent)
{
    _type = "radiogroup";
    setCopyPasteEnabled(false);
    qbuttongroup = new QButtonGroup(this);
    qhboxlayout = new QHBoxLayout();
    this->setLayout(qhboxlayout);
}

void RadioGroupGUI::addButton(const std::string& key, RadioButtonGUI* newWidget) {
    if (_addedKeys.find(key) != _addedKeys.end()) {
        return;
    }

    newWidget->setParent(this);
    qbuttongroup->addButton(newWidget);
    qhboxlayout->addWidget(newWidget);
    newWidget->setKey(key);
    _addedKeys.insert(key);
}

void RadioGroupGUI::getLuaValue(kaguya::LuaRef& table) {
    RadioButtonGUI* checkButton = qobject_cast<RadioButtonGUI*>(qbuttongroup->checkedButton());
    if (checkButton != nullptr) {
        table[_key] = checkButton->label();
    }

    QList<QAbstractButton*> buttons = qbuttongroup->buttons();
    for (QAbstractButton* abstractButton : buttons) {
        RadioButtonGUI* guiButton = qobject_cast<RadioButtonGUI*>(abstractButton);
        if (guiButton != nullptr) {
            guiButton->getLuaValue(table);
        }
    }
}

std::set<std::string> RadioGroupGUI::getKeys() {
    return _addedKeys;
}

void RadioGroupGUI::copyValue(QDataStream& stream) {
}

void RadioGroupGUI::pasteValue(QDataStream& stream) {
}

void RadioGroupGUI::hideLabel() {
}
