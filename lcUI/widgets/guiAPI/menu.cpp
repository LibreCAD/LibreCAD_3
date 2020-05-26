#include "menu.h"

using namespace lc::ui::api;

Menu::Menu(const char* menuName, QWidget* parent)
    :
    QMenu(QString(menuName), parent)
{
    this->setObjectName(menuName);
}

Menu::Menu(QMenuBar* menuBar)
    :
    QMenu(menuBar)
{
}

Menu::Menu(QMenu* menu)
    :
    QMenu(menu)
{
}

void Menu::addItem(MenuItem* item) {
    this->addAction(item);
}

const std::string& Menu::getLabel() {
    return this->title().toStdString();
}
