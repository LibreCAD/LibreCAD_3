#include "inputguicontainer.h"

#include "horizontalgroupgui.h"
#include "radiogroupgui.h"
#include "coordinategui.h"
#include "entitygui.h"
#include "listgui.h"

#include <iostream>

using namespace lc::ui::api;

InputGUIContainer::InputGUIContainer(const std::string& label, lc::ui::MainWindow* mainWindow)
    :
    mainWindow(mainWindow),
    _label(label)
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
    ListGUI* isListGUI = qobject_cast<ListGUI*>(guiWidget);

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

    if (isListGUI != nullptr) {
        isListGUI->setMainWindow(mainWindow);
    }

    guiWidget->setKey(key);
    _inputWidgets[key] = guiWidget;
    _addedKeys.insert(key);
    return true;
}

std::vector<InputGUI*> InputGUIContainer::inputWidgets() {
    std::vector<InputGUI*> inputGUIList;

    for (std::map<std::string, InputGUI*>::iterator iter = _inputWidgets.begin(); iter != _inputWidgets.end(); ++iter) {
        inputGUIList.push_back(iter->second);
    }

    return inputGUIList;
}

void InputGUIContainer::addFinishCallback(lc::scripting::ScriptCallback cb) {
    _callbacks.push_back(std::move(cb));
}

lc::scripting::Map InputGUIContainer::generateInfo() {
    auto info = lc::scripting::makeMap();
    for (InputGUI* inputgui : inputWidgets()) {
        if (inputgui != nullptr) {
            inputgui->getValue(info);
        }
    }
    return info;
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

void InputGUIContainer::removeInputGUI(std::string key, bool deleteWidget) {
    HorizontalGroupGUI* isHorizGroup = qobject_cast<HorizontalGroupGUI*>(_inputWidgets[key]);
    RadioGroupGUI* isRadioGroup = qobject_cast<RadioGroupGUI*>(_inputWidgets[key]);

    if (isHorizGroup != nullptr) {
        std::set<std::string> widgetKeys = isHorizGroup->getKeys();

        for (std::string wkey : widgetKeys) {
            _addedKeys.erase(wkey);
        }
    }

    if (isRadioGroup != nullptr) {
        std::set<std::string> widgetKeys = isRadioGroup->getKeys();

        for (std::string wkey : widgetKeys) {
            _addedKeys.erase(wkey);
        }
    }

    if (deleteWidget) {
        delete _inputWidgets[key];
    }
    _inputWidgets.erase(key);
    _addedKeys.erase(key);
}
