#include "toolbargroup.h"

using namespace lc::ui::api;

ToolbarGroup::ToolbarGroup(const char* groupName, int width, QWidget* parent) 
    :
    QGroupBox(groupName, parent),
    width(width),
    luaInterface(nullptr)
{
    this->setLayout(new QGridLayout());
}

void ToolbarGroup::addButton(ToolbarButton* button) {
    auto gridLayout = static_cast<QGridLayout*>(this->layout());
    gridLayout->addWidget(button, 0, 0, 1, 1);

    if (luaInterface != nullptr) {
        button->setLuaInterface(luaInterface);
    }
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
