#include "horizontalgroupgui.h"

using namespace lc::ui::api;

HorizontalGroupGUI::HorizontalGroupGUI(std::string label, QWidget* parent)
    :
    InputGUI(label, parent)
{
    qhboxlayout = new QHBoxLayout();
    this->setLayout(qhboxlayout);
}

void HorizontalGroupGUI::addWidget(QWidget* newWidget) {
    newWidget->setParent(this);
    _widgets.push_back(newWidget);
    qhboxlayout->addWidget(newWidget);
}
