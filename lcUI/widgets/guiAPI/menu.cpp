#include "menu.h"

using namespace lc::ui::api;

Menu::Menu(const char* menuName, QWidget* parent)
    :
    QMenu(QString(menuName), parent),
    luaInterface(nullptr),
    menuPosition(-1)
{
    this->setObjectName(menuName);
}

Menu::Menu(QMenuBar* menuBar)
    :
    QMenu(menuBar),
    luaInterface(nullptr),
    menuPosition(-1)
{
}

Menu::Menu(QMenu* menu)
    :
    QMenu(menu),
    luaInterface(nullptr),
    menuPosition(-1)
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
    this->addAction(newItem);

    if (luaInterface != nullptr) {
        newItem->setLuaInterface(luaInterface);
    }

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
    menuPosition = menuList.size() - 1;
}

int Menu::getPosition() const {
    return menuPosition;
}

void Menu::setPosition(int newPosition) {
    if (newPosition == menuPosition || newPosition < 0) {
        return;
    }
    int setToNewPosition = newPosition;

    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();
    QMenuBar* menuBar = static_cast<QMenuBar*>(widgets[0]);

    QList<QAction*> menuList = menuBar->actions();

    if (newPosition > menuPosition) {
        newPosition++;
    }

    menuBar->insertMenu(menuList[newPosition], this);
    menuPosition = setToNewPosition;

    // update position of menus after new menuPosition
    menuList = menuBar->actions();
    int n = menuList.size();

    for (int i = menuPosition + 1; i < n; i++)
    {
        Menu* item = static_cast<Menu*>(menuList[i]->menu());
        item->updatePositionVariable(i);
    }
}

void Menu::updatePositionVariable(int newPosition) {
    menuPosition = newPosition;
}
