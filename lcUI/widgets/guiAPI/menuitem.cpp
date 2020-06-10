#include "menuitem.h"

#include "menu.h"

#include <stack>

using namespace lc::ui;
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

std::string MenuItem::label() {
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

void MenuItem::addCallback(const char* cb_name, kaguya::LuaRef callback) {
    if (namedCallbacks.find(cb_name) != namedCallbacks.end()) {
        return;
    }
    
    addCallback(callback);
    namedCallbacks[cb_name] = callbacks.size() - 1;
}

void MenuItem::removeCallback(const char* cb_name) {
    if (luaInterface != nullptr) {
        if (namedCallbacks.find(cb_name) != namedCallbacks.end()) {
            luaInterface->luaDisconnect(this, "triggered(bool)", callbacks[namedCallbacks[cb_name]]);
            callbacks.erase(callbacks.begin() + namedCallbacks[cb_name]);
            namedCallbacks.erase(namedCallbacks.find(cb_name));
        }
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
    int size = items.size();

    for (int i = 0; i < size; i++)
    {
        QAction* item = items[i];
        if (i > position) {
            if (item->menu()) {
                Menu* menu_i = static_cast<Menu*>(item->menu());
                menu_i->updatePositionVariable(menu_i->getPosition() - 1);
            }
            else {
                MenuItem* menuItem = static_cast<MenuItem*>(item);
                menuItem->updatePositionVariable(menuItem->getPosition() - 1);
            }
        }
    }

    QAction* currentItem = items[position];
    menu->removeAction(currentItem);
    std::stack<QAction*> actionsStack;

    if (newPosition >= size) {
        newPosition = size - 1;
    }

    items = menu->actions();
    while (items.size() != newPosition)
    {
        QAction* item = items[items.size() - 1];
        if (item->menu()) {
            Menu* menu_i = static_cast<Menu*>(item->menu());
            menu_i->updatePositionVariable(items.size());
        }
        else {
            MenuItem* menuItem = static_cast<MenuItem*>(item);
            menuItem->updatePositionVariable(items.size());
        }
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
    updateOtherPositionsAfterRemove();

    QList<QWidget*> widgets = this->associatedWidgets();

    for (QWidget* widget : widgets) {
        QMenu* menu = static_cast<QMenu*>(widget);
        menu->removeAction(this);
    }
}

void MenuItem::updatePositionVariable(int pos) {
    position = pos;
}

void MenuItem::updateOtherPositionsAfterRemove() {
    QList<QWidget*> widgets = this->associatedWidgets();

    for (QWidget* widget : widgets) {
        QMenu* menu = static_cast<QMenu*>(widget);
        
        QList<QAction*> actions = menu->actions();

        for (int i = position + 1; i < actions.size(); i++) {
            QAction* item = actions[i];

            if (item->menu()) {
                Menu* menu_i = static_cast<Menu*>(item->menu());
                menu_i->updatePositionVariable(menu_i->getPosition() - 1);
            }
            else {
                MenuItem* menuItem = static_cast<MenuItem*>(item);
                menuItem->updatePositionVariable(menuItem->getPosition() - 1);
            }
        }
    }
}
