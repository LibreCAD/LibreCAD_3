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

    if (_checkable) {
        connect(this, &ToolbarButton::toggled, this, &ToolbarButton::callbackCalledToggle);
    }
    else {
        connect(this, &ToolbarButton::pressed, this, &ToolbarButton::callbackCalled);
    }
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

void ToolbarButton::addCallback(const char* cb_name, kaguya::LuaRef callback) {
    namedCallbacks[cb_name] = callbacks.size();
    addCallback(callback);
}

void ToolbarButton::removeCallback(const char* cb_name) {
    callbacks.erase(callbacks.begin() + namedCallbacks[cb_name]);
    namedCallbacks.erase(namedCallbacks.find(cb_name));
}

void ToolbarButton::callbackCalled() {
    for (int i = 0; i < callbacks.size(); i++) {
        callbacks[i]();
    }
}

void ToolbarButton::callbackCalledToggle(bool enabled) {
    for (int i = 0; i < callbacks.size(); i++) {
        callbacks[i](enabled);
    }
}
