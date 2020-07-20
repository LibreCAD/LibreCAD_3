#include "listgui.h"
#include "ui_listgui.h"

#include "coordinategui.h"
#include "entitygui.h"

using namespace lc::ui::api;

ListGUI::ListGUI(std::string label, QWidget* parent)
    : 
    InputGUI(label, parent),
    ui(new Ui::ListGUI),
    mainWindow(nullptr)
{
    ui->setupUi(this);

    listWidget = qobject_cast<QListWidget*>(ui->verticalLayout->itemAt(1)->widget());
    qobject_cast<QLabel*>(ui->verticalLayout->itemAt(0)->widget())->setText(label.c_str());
}

ListGUI::~ListGUI()
{
    delete ui;
}

void ListGUI::getLuaValue(kaguya::LuaRef& table) {
    table[_key] = kaguya::NewTable();

    for (InputGUI* inputWidget : itemList) {
        if (inputWidget != nullptr) {
            inputWidget->getLuaValue(table);
            table[_key][inputWidget->key()] = table[inputWidget->key()];
        }
    }
}

void ListGUI::addItem(const std::string& key, InputGUI* newitem) {
    if (_addedKeys.find(key) != _addedKeys.end()) {
        return;
    }

    newitem->setKey(key);

    CoordinateGUI* coordgui = qobject_cast<CoordinateGUI*>(newitem);
    EntityGUI* entitygui = qobject_cast<EntityGUI*>(newitem);

    if (coordgui != nullptr) {
        coordgui->enableCoordinateSelection(mainWindow);
    }

    if (entitygui != nullptr) {
        entitygui->enableWidgetSelection(mainWindow);
    }

    QListWidgetItem* itemW = new QListWidgetItem();
    itemW->setSizeHint(newitem->sizeHint());
    listWidget->addItem(itemW);
    listWidget->setItemWidget(itemW, newitem);
    itemList.push_back(newitem);
}

void ListGUI::setMainWindow(lc::ui::MainWindow* mainWindowIn) {
    mainWindow = mainWindowIn;
}

void ListGUI::setLabel(const std::string& newLabel) {
    qobject_cast<QLabel*>(ui->verticalLayout->itemAt(0)->widget())->setText(newLabel.c_str());
    InputGUI::setLabel(newLabel);
}

std::set<std::string> ListGUI::getKeys() const {
    return _addedKeys;
}
