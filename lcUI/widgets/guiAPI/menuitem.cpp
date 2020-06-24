#include "menuitem.h"

#include "menu.h"
#include <stack>

using namespace lc::ui;
using namespace lc::ui::api;

MenuItem::MenuItem(const char* menuItemName, kaguya::LuaRef callback, QWidget* parent)
    :
    MenuItem(menuItemName, parent)
{
    callbacks.push_back(callback);
}

MenuItem::MenuItem(const char* menuItemName, QWidget* parent)
    :
    QAction(menuItemName, parent),
    _position(-1)
{
    this->setObjectName(menuItemName);
    connect(this, &MenuItem::triggered, this, &MenuItem::itemTriggered);
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

void MenuItem::addCallback(kaguya::LuaRef callback) {
    callbacks.push_back(callback);
}

void MenuItem::addCallback(const char* cb_name, kaguya::LuaRef callback) {
    if (namedCallbacks.find(cb_name) != namedCallbacks.end()) {
        return;
    }
    
    addCallback(callback);
    namedCallbacks[cb_name] = callbacks.size() - 1;
}

void MenuItem::removeCallback(const char* cb_name) {
    if (namedCallbacks.find(cb_name) != namedCallbacks.end()) {
        callbacks.erase(callbacks.begin() + namedCallbacks[cb_name]);
        namedCallbacks.erase(namedCallbacks.find(cb_name));
    }
}

int MenuItem::position() const {
    return _position;
}

void MenuItem::setPosition(int newPosition) {
    if (newPosition == _position) {
        return;
    }

    if (newPosition < 0) {
        newPosition = 0;
    }

    QList<QWidget*> widgets = this->associatedWidgets();
    QMenu* menu = qobject_cast<QMenu*>(widgets[0]);

    if (menu == nullptr) {
        return;
    }

    QList<QAction*> items = menu->actions();
    updateItemPositionsAfterSet(items);

    QAction* currentItem = items[_position];
    menu->removeAction(currentItem);
    std::stack<QAction*> actionsStack;

    if (newPosition >= items.size()) {
        newPosition = items.size() - 1;
    }

    items = menu->actions();
    while (items.size() != newPosition)
    {
        QAction* item = items[items.size() - 1];
        if (item->menu()) {
            Menu* menu_i = qobject_cast<Menu*>(item->menu());
            if (menu_i != nullptr) {
                menu_i->updatePositionVariable(items.size());
            }
        }
        else {
            MenuItem* menuItem = qobject_cast<MenuItem*>(item);
            if (menuItem != nullptr) {
                menuItem->updatePositionVariable(items.size());
            }
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

    _position = newPosition;
}

void MenuItem::remove() {
    updateOtherPositionsAfterRemove();

    QList<QWidget*> widgets = this->associatedWidgets();

    for (QWidget* widget : widgets) {
        QMenu* menu = qobject_cast<QMenu*>(widget);
        if (menu != nullptr) {
            menu->removeAction(this);
        }
    }
}

void MenuItem::updatePositionVariable(int pos) {
    _position = pos;
}

void MenuItem::updateOtherPositionsAfterRemove() {
    QList<QWidget*> widgets = this->associatedWidgets();

    for (QWidget* widget : widgets) {
        QMenu* menu = qobject_cast<QMenu*>(widget);
        
        if (menu == nullptr) {
            return;
        }

        QList<QAction*> actions = menu->actions();

        for (int i = _position + 1; i < actions.size(); i++) {
            QAction* item = actions[i];

            if (item->menu()) {
                Menu* menu_i = qobject_cast<Menu*>(item->menu());
                if (menu_i != nullptr) {
                    menu_i->updatePositionVariable(menu_i->position() - 1);
                }
            }
            else {
                MenuItem* menuItem = qobject_cast<MenuItem*>(item);
                if (menuItem != nullptr) {
                    menuItem->updatePositionVariable(menuItem->position() - 1);
                }
            }
        }
    }
}

void MenuItem::itemTriggered() {
    for (int i = 0; i < callbacks.size(); i++) {
        callbacks[i]();
    }
}

void MenuItem::updateItemPositionsAfterSet(QList<QAction*>& items) {
    int size = items.size();

    for (int i = 0; i < size; i++)
    {
        QAction* item = items[i];
        if (i > _position) {
            if (item->menu()) {
                Menu* menu_i = qobject_cast<Menu*>(item->menu());
                if (menu_i != nullptr) {
                    menu_i->updatePositionVariable(menu_i->position() - 1);
                }
            }
            else {
                MenuItem* menuItem = qobject_cast<MenuItem*>(item);
                if (menuItem != nullptr) {
                    menuItem->updatePositionVariable(menuItem->position() - 1);
                }
            }
        }
    }
}
