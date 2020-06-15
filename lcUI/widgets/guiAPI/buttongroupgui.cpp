#include "buttongroupgui.h"

using namespace lc::ui::api;

ButtonGroupGUI::ButtonGroupGUI(std::string label, QWidget* parent)
    :
    InputGUI(label, parent)
{
    qhboxlayout = new QHBoxLayout();
    this->setLayout(qhboxlayout);
}

void ButtonGroupGUI::addButton(ButtonGUI* newButton) {
    newButton->setParent(this);
    _buttons.push_back(newButton);
    qhboxlayout->addWidget(newButton);
}
