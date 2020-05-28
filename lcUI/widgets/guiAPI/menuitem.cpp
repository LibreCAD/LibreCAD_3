#include "menuitem.h"

#include <stack>

using namespace lc::ui::api;

MenuItem::MenuItem(const char* menuItemName, kaguya::LuaRef callback, QWidget* parent)
    :
    QAction(menuItemName, parent),
    luaInterface(nullptr),
    position(-1)
{
    this->setObjectName(menuItemName);
    callbacks.push_back(callback);
}

MenuItem::MenuItem(const char* menuItemName, QWidget* parent)
    :
    QAction(menuItemName, parent),
    luaInterface(nullptr),
    position(-1)
{
    this->setObjectName(menuItemName);
}

std::string MenuItem::getLabel() {
    return this->text().toStdString();
}

void MenuItem::setLabel(const char* newLabel) {
    this->setText(newLabel);
}

void MenuItem::hide() {
    setVisible(false);
}

void MenuItem::show() {
    setVisible(true);
}

void MenuItem::setLuaInterface(LuaInterface* luaInterfaceIn, bool setCallbacks) {
    if (luaInterface != nullptr) {
        return;
    }

    luaInterface = luaInterfaceIn;

    // loop through all already added callbacks and connect them
    if (setCallbacks) {
        for (kaguya::LuaRef cb : callbacks)
        {
            luaInterface->luaConnect(this, "triggered(bool)", cb);
        }
    }

    //determine position of menu item
    QList<QWidget*> widgets = this->associatedWidgets();
    QMenu* menu = static_cast<QMenu*>(widgets[0]);

    QList<QAction*> items = menu->actions();

    for (int i = 0; i < items.size(); i++){
        if (items[i] == this) {
            position = i;
            break;
        }
    }
}

void MenuItem::addCallback(kaguya::LuaRef callback) {
    callbacks.push_back(callback);

    if (luaInterface != nullptr) {
        luaInterface->luaConnect(this, "triggered(bool)", callback);
    }
}

int MenuItem::getPosition() const {
    return position;
}

void MenuItem::setPosition(int newPosition) {
    if (newPosition == position) {
        return;
    }

    if (newPosition < 0) {
        newPosition = 0;
    }

    QList<QWidget*> widgets = this->associatedWidgets();
    QMenu* menu = static_cast<QMenu*>(widgets[0]);

    QList<QAction*> items = menu->actions();

    QAction* currentItem = items[position];

    menu->removeAction(currentItem);

    std::stack<QAction*> actionsStack;

    items = menu->actions();
    while (items.size() != newPosition)
    {
        QAction* item = items[items.size() - 1];
        MenuItem* menuItem = static_cast<MenuItem*>(item);
        menuItem->updatePositionVariable(items.size());
        actionsStack.push(item);
        menu->removeAction(item);
        items = menu->actions();
    }

    menu->addAction(currentItem);

    while (actionsStack.size() > 0) {
        menu->addAction(actionsStack.top());
        actionsStack.pop();
    }

    position = newPosition;
}

void MenuItem::remove() {
    QList<QWidget*> widgets = this->associatedWidgets();

    for (QWidget* widget : widgets) {
        QMenu* menu = static_cast<QMenu*>(widget);
        menu->removeAction(this);
    }
}

void MenuItem::updatePositionVariable(int pos) {
    position = pos;
}
