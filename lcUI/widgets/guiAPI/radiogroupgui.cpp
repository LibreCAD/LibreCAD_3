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
