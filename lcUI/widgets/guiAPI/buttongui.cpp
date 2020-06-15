#include "buttongui.h"

using namespace lc::ui::api;

ButtonGUI::ButtonGUI(std::string label, QWidget* parent)
    :
    QPushButton(QString(label.c_str()), parent),
    _label(label)
{
}
