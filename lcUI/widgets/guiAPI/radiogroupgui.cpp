#include "radiogroupgui.h"

using namespace lc::ui::api;

RadioGroupGUI::RadioGroupGUI(std::string label, QWidget* parent)
    :
    InputGUI(label, parent)
{
    qbuttongroup = new QButtonGroup(this);
    qhboxlayout = new QHBoxLayout();
    this->setLayout(qhboxlayout);
}

void RadioGroupGUI::addButton(RadioButtonGUI* newWidget) {
    newWidget->setParent(this);
    qbuttongroup->addButton(newWidget);
    qhboxlayout->addWidget(newWidget);
}

void RadioGroupGUI::getLuaValue(kaguya::LuaRef& table) {
    RadioButtonGUI* checkButton = qobject_cast<RadioButtonGUI*>(qbuttongroup->checkedButton());
    if (checkButton != nullptr) {
        table[label()] = checkButton->label();
    }
    
    QList<QAbstractButton*> buttons = qbuttongroup->buttons();
    for (QAbstractButton* abstractButton : buttons){
        RadioButtonGUI* guiButton = qobject_cast<RadioButtonGUI*>(abstractButton);
        if (guiButton != nullptr) {
            guiButton->getLuaValue(table);
        }
    }
}
