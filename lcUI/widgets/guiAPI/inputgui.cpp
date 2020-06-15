#include "inputgui.h"

using namespace lc::ui::api;

InputGUI::InputGUI(std::string _label, QWidget* parent)
    :
    QWidget(parent),
    _label(_label){

}

const std::string& InputGUI::label() const {
    return _label;
}

void InputGUI::setLabel(const std::string& newLabel) {
    _label = newLabel;
}
