#include "horizontalgroupgui.h"

#include "buttongui.h"
#include "checkboxgui.h"
#include "radiobuttongui.h"

using namespace lc::ui::api;

HorizontalGroupGUI::HorizontalGroupGUI(std::string label, bool vertical, QWidget* parent)
    :
    InputGUI(label, parent)
{
    if (vertical) {
        qboxlayout = new QVBoxLayout();
    }
    else {
        qboxlayout = new QHBoxLayout();
    }
    this->setLayout(qboxlayout);
}

void HorizontalGroupGUI::addWidget(const std::string& key, QWidget* newWidget) {
    if (_addedKeys.find(key) != _addedKeys.end() || _buttonKeys.find(key) != _buttonKeys.end()) {
        return;
    }

    InputGUI* inpgui = qobject_cast<InputGUI*>(newWidget);
    if (inpgui != nullptr) {
        inpgui->setKey(key);
    }

    CheckBoxGUI* checkbox = qobject_cast<CheckBoxGUI*>(newWidget);
    if (checkbox != nullptr) {
        checkbox->setKey(key);
    }

    RadioButtonGUI* radiobutton = qobject_cast<RadioButtonGUI*>(newWidget);
    if (radiobutton != nullptr) {
        radiobutton->setKey(key);
    }

    ButtonGUI* button = qobject_cast<ButtonGUI*>(newWidget);
    if (button != nullptr) {
        _buttonKeys.insert(key);
    }
    else {
        _addedKeys.insert(key);
    }

    newWidget->setParent(this);
    _widgets.push_back(newWidget);
    qboxlayout->addWidget(newWidget);
}

void HorizontalGroupGUI::getLuaValue(kaguya::LuaRef& table) {
    for (QWidget* inputWidget : _widgets) {
        InputGUI* inpgui = qobject_cast<InputGUI*>(inputWidget);
        if (inpgui != nullptr) {
            inpgui->getLuaValue(table);
        }else {
            CheckBoxGUI* checkbox = qobject_cast<CheckBoxGUI*>(inputWidget);
            if (checkbox != nullptr) {
                checkbox->getLuaValue(table);
            }

            RadioButtonGUI* radiobutton = qobject_cast<RadioButtonGUI*>(inputWidget);
            if (radiobutton != nullptr) {
                radiobutton->getLuaValue(table);
            }
        }
    }
}

std::set<std::string> HorizontalGroupGUI::getKeys() {
    return _addedKeys;
}
