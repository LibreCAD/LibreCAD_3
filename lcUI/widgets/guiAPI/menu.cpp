#include "menu.h"

#include <mainwindow.h>

using namespace lc::ui::api;

Menu::Menu(const char* menuName, QWidget* parent)
    :
    QMenu(QString(menuName), parent),
    luaInterface(nullptr),
    position(-1)
{
    this->setObjectName(menuName);
}

Menu::Menu(QMenuBar* menuBar)
    :
    QMenu(menuBar),
    luaInterface(nullptr),
    position(-1)
{
}

Menu::Menu(QMenu* menu)
    :
    QMenu(menu),
    luaInterface(nullptr),
    position(-1)
{
}

void Menu::addItem(MenuItem* item) {
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

std::string Menu::getLabel() {
    return this->title().toStdString();
}

void Menu::setLabel(const char* newMenuLabel) {
    this->setTitle(newMenuLabel);
}

MenuItem* Menu::getItem(const char* menuItemLabel) {
    QList<QAction*> menuActions = this->actions();

    for (QAction* action : menuActions)
    {
        MenuItem* item = static_cast<MenuItem*>(action);

        if (item->getLabel() == std::string(menuItemLabel)) {
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

    return static_cast<MenuItem*>(menuActions[pos]);
}

void Menu::hide() {
    menuAction()->setVisible(false);
}

void Menu::show() {
    menuAction()->setVisible(true);
}

void Menu::removeItem(const char* menuItemLabel) {
    removeAction(getItem(menuItemLabel));
}

void Menu::removeItem(MenuItem* item) {
    removeAction(item);
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
        MenuItem* item = static_cast<MenuItem*>(action);
        item->setLuaInterface(luaInterface, setCallbacks);
    }

    /*
     * luaInterface is connected when the menu gets added, so determine
     * the position of the menu
    */
    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();
    QMenuBar* menuBar = static_cast<QMenuBar*>(widgets[0]);
    QList<QAction*> menuList = menuBar->actions();
    position = menuList.size() - 1;
}

int Menu::getPosition() const {
    return position;
}

void Menu::setPosition(int newPosition) {
    if (newPosition == position || newPosition < 0) {
        return;
    }
    int setToNewPosition = newPosition;

    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();
    QMenuBar* menuBar = static_cast<QMenuBar*>(widgets[0]);

    QList<QAction*> menuList = menuBar->actions();

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

void Menu::remove() {
    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();
    QMenuBar* menuBar = static_cast<QMenuBar*>(widgets[0]);

    lc::ui::MainWindow* mainWindow = static_cast<lc::ui::MainWindow*>(menuBar->parentWidget());

    mainWindow->removeFromMenuMap(this->getLabel());
    menuBar->removeAction(this->menuAction());
}

void Menu::updatePositionVariable(int newPosition) {
    position = newPosition;
}
