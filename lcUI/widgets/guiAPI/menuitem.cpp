#include "menuitem.h"

using namespace lc::ui::api;

MenuItem::MenuItem(const char* menuItemName, kaguya::LuaRef callback, QWidget* parent)
    :
    QAction(menuItemName, parent)
{
    this->setObjectName(menuItemName);
}

MenuItem::MenuItem(const char* menuItemName, QWidget* parent)
    :
    QAction(menuItemName, parent)
{
    this->setObjectName(menuItemName);
}

std::string MenuItem::getLabel() {
    return this->text().toStdString();
}
