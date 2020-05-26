#include "menu.h"

using namespace lc::ui::api;

Menu::Menu(const char* menuName, QWidget* parent)
    :
    QMenu(QString(menuName), parent),
    luaInterface(nullptr)
{
    this->setObjectName(menuName);
}

Menu::Menu(QMenuBar* menuBar)
    :
    QMenu(menuBar),
    luaInterface(nullptr)
{
}

Menu::Menu(QMenu* menu)
    :
    QMenu(menu),
    luaInterface(nullptr)
{
}

void Menu::addItem(MenuItem* item) {
    this->addAction(item);

    if (luaInterface != nullptr) {
        item->setLuaInterface(luaInterface);
    }
}

MenuItem* Menu::addItem(const char* menuItemName) {
    MenuItem* newItem = new MenuItem(menuItemName);
    this->addAction(newItem);

    if (luaInterface != nullptr) {
        newItem->setLuaInterface(luaInterface);
    }

    return newItem;
}

std::string Menu::getLabel() {
    return this->title().toStdString();
}

MenuItem* Menu::getMenuItem(const char* menuItemLabel) {
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

void Menu::setLuaInterface(LuaInterface* luaInterfaceIn) {
    if (luaInterface != nullptr) {
        return;
    }

    luaInterface = luaInterfaceIn;

    // set luaInterface of all child menu items
    QList<QAction*> menuActions = this->actions();

    for (QAction* action : menuActions)
    {
        MenuItem* item = static_cast<MenuItem*>(action);
        item->setLuaInterface(luaInterface);
    }
}
