#include "dialogwidget.h"
#include "ui_dialogwidget.h"

#include "horizontalgroupgui.h"
#include "radiogroupgui.h"
#include "coordinategui.h"
#include "entitygui.h"

#include <iostream>
#include <QGridLayout>

using namespace lc::ui::api;

DialogWidget::DialogWidget(const std::string& dialogTitle, lc::ui::MainWindow* mainWindow, QWidget* parent)
    : 
    QDialog(parent),
    mainWindow(mainWindow),
    ui(new Ui::DialogWidget)
{
    ui->setupUi(this);
    this->setWindowTitle(QString(dialogTitle.c_str()));
    show();

    auto gridLayout = qobject_cast<QGridLayout*>(this->layout());
    vboxlayout = gridLayout->findChild<QVBoxLayout*>("verticalLayout");
}

DialogWidget::~DialogWidget()
{
    delete ui;
}

void DialogWidget::setTitle(const std::string& newTitle) {
    this->setWindowTitle(QString(newTitle.c_str()));
}

std::string DialogWidget::title() const {
    return this->windowTitle().toStdString();
}

bool DialogWidget::addWidget(const std::string& key, InputGUI* guiWidget) {
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
    guiWidget->setParent(this);
    vboxlayout->addWidget(guiWidget);
    _inputWidgets.push_back(guiWidget);
    _addedKeys.insert(key);
    return true;
}

bool DialogWidget::addWidget(const std::string& key, ButtonGUI* buttonWidget) {
    if (buttonWidget == nullptr) {
        return false;
    }

    HorizontalGroupGUI* horizGroup = new HorizontalGroupGUI(key + "_group");
    horizGroup->addWidget(key, buttonWidget);
    addWidget(horizGroup->label(), horizGroup);
    return true;
}

bool DialogWidget::addWidget(const std::string& key, CheckBoxGUI* checkboxWidget) {
    if (checkboxWidget == nullptr) {
        return false;
    }

    HorizontalGroupGUI* horizGroup = new HorizontalGroupGUI(key + "_group");
    horizGroup->addWidget(key, checkboxWidget);
    addWidget(horizGroup->label(), horizGroup);
    return true;
}

const std::vector<InputGUI*>& DialogWidget::inputWidgets() {
    return _inputWidgets;
}

void DialogWidget::setFinishButton(ButtonGUI* buttonWidget) {
    if (buttonWidget != nullptr) {
        connect(buttonWidget, &ButtonGUI::clicked, this, &DialogWidget::finishCallbacks);
    }
}

void DialogWidget::finishCallbacks() {
    for (kaguya::LuaRef& cb : _callbacks) {
        kaguya::LuaRef result = generateDialogInfo(cb.state());
        cb(result);
    }
}

void DialogWidget::addFinishCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

kaguya::LuaRef DialogWidget::generateDialogInfo(lua_State *luastate) {
    kaguya::State state(luastate);
    std::string refName = title() + "_table";
    state[refName] = kaguya::NewTable();
    kaguya::LuaRef table = state[refName];
    std::vector<InputGUI*> inputGUIs = inputWidgets();

    for (InputGUI* inputgui : inputGUIs) {
        inputgui->getLuaValue(table);
    }

    return table;
}

std::vector<std::string> DialogWidget::keys() const {
    std::vector<std::string> listOfKeys;
    for (std::string key : _addedKeys) {
        listOfKeys.push_back(key);
    }
    return listOfKeys;
}
