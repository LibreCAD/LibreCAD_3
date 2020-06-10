#include "toolbarbutton.h"

#include "toolbargroup.h"

using namespace lc::ui::api;

ToolbarButton::ToolbarButton(const char* buttonLabel, const char* icon, kaguya::LuaRef callback, const char* tooltip, bool _checkable, QWidget* parent)
    :
    ToolbarButton(buttonLabel, icon, tooltip, _checkable, parent)
{
    callbacks.push_back(callback);
}

ToolbarButton::ToolbarButton(const char* buttonLabel, const char* icon, const char* tooltip, bool _checkable, QWidget* parent)
    :
    _label(buttonLabel),
    QPushButton("", parent),
    _connected(false),
    _checkable(_checkable)
{
    this->setObjectName(buttonLabel);
    if (tooltip == "") {
        this->setToolTip(buttonLabel);
    }
    else {
        this->setToolTip(tooltip);
    }
    this->setFlat(true);

    if (_checkable) {
        this->setCheckable(true);
    }

    changeIcon(icon);
}

std::string ToolbarButton::label() const {
    return _label;
}

void ToolbarButton::setLabel(const char* newLabel) {
    _label = std::string(newLabel);
}

void ToolbarButton::setTooltip(const char* newToolTip) {
    this->setToolTip(newToolTip);
}

void ToolbarButton::addCallback(kaguya::LuaRef callback) {
    callbacks.push_back(callback);

    if (_connected) {
        emit connectToCallback(callbacks.size() - 1, this);
    }
}

void ToolbarButton::changeIcon(const char* icon) {
    if (icon != nullptr) {
        this->setIcon(QIcon(icon));
        this->setIconSize(QSize(24, 24));
    }
}

void ToolbarButton::remove() {
    emit removeButton(this);
}

bool ToolbarButton::checkable() const {
    return _checkable;
}

kaguya::LuaRef& ToolbarButton::getCallback(int index) {
    return callbacks[index];
}

void ToolbarButton::enableConnections() {
    if (_connected) {
        return;
    }

    _connected = true;

    // loop through all already added callbacks and connect them
    for (int i = 0; i < callbacks.size(); i++)
    {
        emit connectToCallback(i, this);
    }
}

