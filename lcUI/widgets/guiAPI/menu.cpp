#include "menu.h"

#include <mainwindow.h>

using namespace lc::ui::api;

Menu::Menu(const char* menuName, QWidget* parent)
    :
    QMenu(QString(menuName), parent),
    luaInterface(nullptr),
    position(-1),
    insideMenu(false)
{
    this->setObjectName(menuName);
}

Menu::Menu(QMenuBar* menuBar)
    :
    QMenu(menuBar),
    luaInterface(nullptr),
    position(-1),
    insideMenu(false)
{
}

Menu::Menu(QMenu* menu)
    :
    QMenu(menu),
    luaInterface(nullptr),
    position(-1),
    insideMenu(false)
{
}

void Menu::addItem(MenuItem* item) {
    if (checkForItemOfSameLabel(item->label().c_str(), false)) {
        return;
    }

    this->addAction(item);

    if (luaInterface != nullptr) {
        item->setLuaInterface(luaInterface);
    }
}

MenuItem* Menu::addItem(const char* menuItemLabel) {
    MenuItem* newItem = new MenuItem(menuItemLabel);
    addItem(newItem);

    return newItem;
}

MenuItem* Menu::addItem(const char* menuItemLabel, kaguya::LuaRef callback) {
    MenuItem* newItem = new MenuItem(menuItemLabel, callback);
    addItem(newItem);

    return newItem;
}

void Menu::addMenu(Menu* newMenu) {
    if (checkForItemOfSameLabel(newMenu->label().c_str(), true)) {
        return;
    }

    this->addAction(newMenu->menuAction());
    newMenu->insideMenu = true;

    if (luaInterface != nullptr) {
        newMenu->setLuaInterface(luaInterface);
    }
}

Menu* Menu::addMenu(const char* menuLabel) {
    Menu* newMenu = new Menu(menuLabel);
    addMenu(newMenu);
    return newMenu;
}

std::string Menu::label() {
    return this->title().toStdString();
}

void Menu::setLabel(const char* newMenuLabel) {
    this->setTitle(newMenuLabel);
}

MenuItem* Menu::getItem(const char* menuItemLabel) {
    QList<QAction*> menuActions = this->actions();

    for (QAction* action : menuActions)
    {
        if (action->menu()) {
            continue;
        }
        MenuItem* item = static_cast<MenuItem*>(action);

        if (item->label() == std::string(menuItemLabel)) {
            return item;
        }
    }

    return nullptr;
}

MenuItem* Menu::getItem(int pos) {
    QList<QAction*> menuActions = this->actions();

    if (pos < 0 || pos >= menuActions.size()) {
        return nullptr;
    }

    if (menuActions[pos]->menu()) {
        return nullptr;
    }

    return static_cast<MenuItem*>(menuActions[pos]);
}

Menu* Menu::getMenu(const char* menuLabel) {
    QList<QAction*> menuActions = this->actions();

    for (QAction* action : menuActions)
    {
        if (action->menu()) {
            Menu* item = static_cast<Menu*>(action->menu());

            if (item->label() == std::string(menuLabel)) {
                return item;
            }
        }
    }

    return nullptr;
}

Menu* Menu::getMenu(int pos) {
    QList<QAction*> menuActions = this->actions();

    if (pos < 0 || pos >= menuActions.size()) {
        return nullptr;
    }

    if (!menuActions[pos]->menu()) {
        return nullptr;
    }

    return static_cast<Menu*>(menuActions[pos]->menu());
}

void Menu::hide() {
    menuAction()->setVisible(false);
}

void Menu::show() {
    menuAction()->setVisible(true);
}

void Menu::removeItem(const char* menuItemLabel) {
    getItem(menuItemLabel)->remove();
}

void Menu::removeItem(MenuItem* item) {
    item->remove();
}

void Menu::removeMenu(const char* menuLabel) {
    getMenu(menuLabel)->remove();
}

void Menu::removeMenu(Menu* menu) {
    menu->remove();
}

void Menu::setLuaInterface(LuaInterface* luaInterfaceIn, bool setCallbacks) {
    if (luaInterface != nullptr) {
        return;
    }

    luaInterface = luaInterfaceIn;

    // set luaInterface of all child menu items
    QList<QAction*> menuActions = this->actions();

    for (QAction* action : menuActions)
    {
        if (action->menu()) {
            Menu* item = static_cast<Menu*>(action->menu());
            item->setLuaInterface(luaInterface, setCallbacks);
            continue;
        }

        MenuItem* item = static_cast<MenuItem*>(action);
        item->setLuaInterface(luaInterface, setCallbacks);
    }

    /*
     * luaInterface is connected when the menu gets added, so determine
     * the position of the menu
    */
    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();

    if (insideMenu) {
        QMenu* menu = static_cast<QMenu*>(widgets[0]);
        QList<QAction*> menuList = menu->actions();
        position = menuList.size() - 1;
    }
    else {
        QMenuBar* menuBar = static_cast<QMenuBar*>(widgets[0]);
        QList<QAction*> menuList = menuBar->actions();
        position = menuList.size() - 1;
    }
}

int Menu::getPosition() const {
    return position;
}

void Menu::setPosition(int newPosition) {
    if (newPosition == position || newPosition < 0) {
        return;
    }

    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();

    if (!insideMenu) {
        QMenuBar* menuBar = static_cast<QMenuBar*>(widgets[0]);

        QList<QAction*> menuList = menuBar->actions();

        if (newPosition >= menuList.size()) {
            newPosition = menuList.size() - 1;
        }

        int setToNewPosition = newPosition;

        if (newPosition > position) {
            newPosition++;
        }

        menuBar->insertMenu(menuList[newPosition], this);
        position = setToNewPosition;

        // update position of menus after new position
        menuList = menuBar->actions();
        int n = menuList.size();

        for (int i = position + 1; i < n; i++)
        {
            Menu* item = static_cast<Menu*>(menuList[i]->menu());
            item->updatePositionVariable(i);
        }
    }
    else {
        QMenu* menu = static_cast<QMenu*>(widgets[0]);

        QList<QAction*> items = menu->actions();

        if (newPosition >= items.size()) {
            newPosition = items.size() - 1;
        }

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
}

void Menu::remove() {
    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();

    updateOtherPositionsAfterRemove();

    if (insideMenu) {
        for (QWidget* widget : widgets) {
            QMenu* parentMenu = static_cast<QMenu*>(widget);
            parentMenu->removeAction(this->menuAction());
        }
    }
    else {
        QMenuBar* menuBar = static_cast<QMenuBar*>(widgets[0]);
        lc::ui::MainWindow* mainWindow = static_cast<lc::ui::MainWindow*>(menuBar->parentWidget());
        mainWindow->removeFromMenuMap(this->label());
        menuBar->removeAction(this->menuAction());
    }
}

void Menu::updatePositionVariable(int newPosition) {
    position = newPosition;
}

bool Menu::checkForItemOfSameLabel(const char* label, bool isMenu) {
    QList<QAction*> menuActions = this->actions();

    for (QAction* action : menuActions)
    {
        if (!isMenu) {
            if (action->menu()) {
                continue;
            }
            MenuItem* item = static_cast<MenuItem*>(action);

            if (item->label() == std::string(label)) {
                return true;
            }
        }
        else {
            if (action->menu()) {
                Menu* item = static_cast<Menu*>(action->menu());

                if (item->label() == std::string(label)) {
                    return true;
                }
            }
        }
    }

    return false;
}

void Menu::updateOtherPositionsAfterRemove() {
    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();

    if (insideMenu) {
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
    else {
        QMenuBar* menuBar = static_cast<QMenuBar*>(widgets[0]);

        QList<QAction*> menuList = menuBar->actions();

        for (int i = position + 1; i < menuList.size(); i++) {
            QAction* item = menuList[i];

            Menu* menu = static_cast<Menu*>(item->menu());
            menu->updatePositionVariable(menu->getPosition() - 1);
        }
    }
}
