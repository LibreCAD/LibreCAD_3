#include "inputguicontainer.h"

#include "horizontalgroupgui.h"
#include "radiogroupgui.h"
#include "coordinategui.h"
#include "entitygui.h"

#include <iostream>

using namespace lc::ui::api;

InputGUIContainer::InputGUIContainer(const std::string& label, lc::ui::MainWindow* mainWindow)
    :
    _label(label),
    mainWindow(mainWindow)
{
}

InputGUIContainer::~InputGUIContainer() {

}

bool InputGUIContainer::addWidget(const std::string& key, InputGUI* guiWidget) {
    if (guiWidget == nullptr) {
        return false;
    }

    if (_addedKeys.find(key) != _addedKeys.end()) {
        return false;
    }

    HorizontalGroupGUI* isHorizGroup = qobject_cast<HorizontalGroupGUI*>(guiWidget);
    RadioGroupGUI* isRadioGroup = qobject_cast<RadioGroupGUI*>(guiWidget);
    CoordinateGUI* isCoordinate = qobject_cast<CoordinateGUI*>(guiWidget);
    EntityGUI* isEntityGUI = qobject_cast<EntityGUI*>(guiWidget);

    if (isHorizGroup != nullptr) {
        std::set<std::string> widgetKeys = isHorizGroup->getKeys();

        for (std::string wkey : widgetKeys) {
            if (_addedKeys.find(wkey) != _addedKeys.end()) {
                return false;
            }
            else {
                _addedKeys.insert(wkey);
            }
        }
    }

    if (isRadioGroup != nullptr) {
        std::set<std::string> widgetKeys = isRadioGroup->getKeys();

        for (std::string wkey : widgetKeys) {
            if (_addedKeys.find(wkey) != _addedKeys.end()) {
                return false;
            }
            else {
                _addedKeys.insert(wkey);
            }
        }
    }

    if (isCoordinate != nullptr) {
        isCoordinate->enableCoordinateSelection(mainWindow);
    }

    if (isEntityGUI != nullptr) {
        isEntityGUI->enableWidgetSelection(mainWindow);
    }

    guiWidget->setKey(key);
    //guiWidget->setParent(this);
    //vboxlayout->addWidget(guiWidget);
    _inputWidgets.push_back(guiWidget);
    _addedKeys.insert(key);
    return true;
}

bool InputGUIContainer::addWidget(const std::string& key, ButtonGUI* buttonWidget) {
    if (buttonWidget == nullptr) {
        return false;
    }

    HorizontalGroupGUI* horizGroup = new HorizontalGroupGUI(key + "_group");
    horizGroup->addWidget(key, buttonWidget);
    addWidget(horizGroup->label(), horizGroup);
    return true;
}

bool InputGUIContainer::addWidget(const std::string& key, CheckBoxGUI* checkboxWidget) {
    if (checkboxWidget == nullptr) {
        return false;
    }

    HorizontalGroupGUI* horizGroup = new HorizontalGroupGUI(key + "_group");
    horizGroup->addWidget(key, checkboxWidget);
    addWidget(horizGroup->label(), horizGroup);
    return true;
}

const std::vector<InputGUI*>& InputGUIContainer::inputWidgets() {
    return _inputWidgets;
}

void InputGUIContainer::addFinishCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

kaguya::LuaRef InputGUIContainer::generateDialogInfo(lua_State* luastate) {
    kaguya::State state(luastate);
    std::string refName = _label + "_table";
    state[refName] = kaguya::NewTable();
    kaguya::LuaRef table = state[refName];
    std::vector<InputGUI*> inputGUIs = inputWidgets();

    for (InputGUI* inputgui : inputGUIs) {
        inputgui->getLuaValue(table);
    }

    return table;
}

std::vector<std::string> InputGUIContainer::keys() const {
    std::vector<std::string> listOfKeys;
    for (std::string key : _addedKeys) {
        listOfKeys.push_back(key);
    }
    return listOfKeys;
}

void InputGUIContainer::setLabel(const std::string& newlabel) {
    _label = newlabel;
}
