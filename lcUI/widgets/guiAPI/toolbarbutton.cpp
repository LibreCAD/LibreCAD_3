#include "toolbarbutton.h"

using namespace lc::ui::api;

ToolbarButton::ToolbarButton(const char* buttonLabel, const char* icon, kaguya::LuaRef callback, const char* tooltip, QWidget* parent)
    :
    ToolbarButton(buttonLabel, icon, tooltip, parent)
{
    callbacks.push_back(callback);
}

ToolbarButton::ToolbarButton(const char* buttonLabel, const char* icon, const char* tooltip, QWidget* parent)
    :
    label(buttonLabel),
    QPushButton("", parent),
    luaInterface(nullptr)
{
    this->setObjectName(buttonLabel);
    if (tooltip == "") {
        this->setToolTip(buttonLabel);
    }
    else {
        this->setToolTip(tooltip);
    }
    this->setFlat(true);

    if (icon != nullptr) {
        this->setIcon(QIcon(icon));
        this->setIconSize(QSize(24, 24));
    }
}

std::string ToolbarButton::getLabel() {
    return label;
}

void ToolbarButton::setLabel(const char* newLabel) {
    label = std::string(newLabel);
}

void ToolbarButton::setTooltip(const char* newToolTip) {
    this->setToolTip(newToolTip);
}

void ToolbarButton::addCallback(kaguya::LuaRef callback) {
    callbacks.push_back(callback);

    if (luaInterface != nullptr) {
        luaInterface->luaConnect(this, "pressed()", callback);
    }
}

void ToolbarButton::setLuaInterface(LuaInterface* luaInterfaceIn) {
    if (luaInterface != nullptr) {
        return;
    }

    luaInterface = luaInterfaceIn;

    // loop through all already added callbacks and connect them
    for (kaguya::LuaRef cb : callbacks)
    {
        luaInterface->luaConnect(this, "pressed()", cb);
    }
}
