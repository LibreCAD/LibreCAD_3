#include "toolbargroup.h"

using namespace lc::ui::api;

ToolbarGroup::ToolbarGroup(const char* groupName, int width, QWidget* parent) 
    :
    QGroupBox(groupName, parent),
    width(width),
    luaInterface(nullptr),
    count(0)
{
    this->setLayout(new QGridLayout());
}

void ToolbarGroup::addButton(ToolbarButton* button) {
    auto gridLayout = static_cast<QGridLayout*>(this->layout());
    gridLayout->addWidget(button, count / width, count % width, 1, 1);

    if (luaInterface != nullptr) {
        button->setLuaInterface(luaInterface);
    }

    count++;
}

ToolbarButton* ToolbarGroup::addButton(const char* buttonName, const char* buttonIcon) {
    ToolbarButton* button = new ToolbarButton(buttonName, buttonIcon);
    addButton(button);
    return button;
}

ToolbarButton* ToolbarGroup::addButton(const char* buttonName, const char* buttonIcon, kaguya::LuaRef callback) {
    ToolbarButton* button = new ToolbarButton(buttonName, buttonIcon, callback);
    addButton(button);
    return button;
}

void ToolbarGroup::addWidget(QWidget* widget) {
    auto gridLayout = static_cast<QGridLayout*>(this->layout());
    gridLayout->addWidget(widget, count / width, count % width, 1, 1);
    count++;
}

std::string ToolbarGroup::label() const {
    return this->title().toStdString();
}

void ToolbarGroup::setLabel(const char* newGroupName) {
    this->setTitle(newGroupName);
}

ToolbarButton* ToolbarGroup::getButton(const char* buttonName) {
    auto nbButtons = this->layout()->count();
    std::string buttonText = std::string(buttonName);

    for (int i = 0; i < nbButtons; i++) {
        auto button = static_cast<ToolbarButton*>(this->layout()->itemAt(i)->widget());

        if (button->label() == buttonText) {
            return button;
        }
    }

    return nullptr;
}

std::vector<ToolbarButton*> ToolbarGroup::getAllButtons() {
    auto nbButtons = this->layout()->count();
    std::vector<ToolbarButton*> buttons;

    for (int i = 0; i < nbButtons; i++) {
        auto button = static_cast<ToolbarButton*>(this->layout()->itemAt(i)->widget());
        buttons.push_back(button);
    }

    return buttons;
}

void ToolbarGroup::removeButton(ToolbarButton* button) {
    auto gridLayout = static_cast<QGridLayout*>(this->layout());
    gridLayout->removeWidget(button);
}

void ToolbarGroup::removeButton(const char* buttonName) {
    removeButton(getButton(buttonName));
}

void ToolbarGroup::setLuaInterface(LuaInterface* luaInterfaceIn) {
    if (luaInterface != nullptr) {
        return;
    }

    luaInterface = luaInterfaceIn;

    auto nbButtons = this->layout()->count();
    for (int i = 0; i < nbButtons; i++) {
        auto button = static_cast<ToolbarButton*>(this->layout()->itemAt(i)->widget());
        button->setLuaInterface(luaInterface);
    }
}
