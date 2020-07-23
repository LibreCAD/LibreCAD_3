#include "listgui.h"
#include "ui_listgui.h"

#include "coordinategui.h"
#include "entitygui.h"
#include "lwvertexgroup.h"

using namespace lc::ui::api;

ListGUI::ListGUI(std::string label, ListGUI::ListType listTypeIn, QWidget* parent)
    : 
    InputGUI(label, parent),
    ui(new Ui::ListGUI),
    mainWindow(nullptr),
    _listType(listTypeIn),
    itemIdCount(0),
    _selectedCoordinate(nullptr)
{
    ui->setupUi(this);

    listWidget = qobject_cast<QListWidget*>(ui->verticalLayout->itemAt(1)->widget());
    qobject_cast<QLabel*>(ui->verticalLayout->itemAt(0)->widget())->setText(label.c_str());
    listWidget->setMinimumHeight(200);

    QLayout* buttonLayout = ui->verticalLayout->itemAt(2)->layout();
    QPushButton* plusButton = qobject_cast<QPushButton*>(buttonLayout->itemAt(0)->widget());
    QPushButton* minusButton = qobject_cast<QPushButton*>(buttonLayout->itemAt(1)->widget());

    connect(plusButton, &QPushButton::clicked, this, &ListGUI::plusButtonClicked);
    connect(minusButton, &QPushButton::clicked, this, &ListGUI::minusButtonClicked);
    connect(listWidget, &QListWidget::currentItemChanged, this, &ListGUI::guiItemChanged);

    if (listTypeIn == ListType::NONE) {
        plusButton->setEnabled(false);
        minusButton->setEnabled(false);
    }
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
    LWVertexGroup* lwVertexGroup = qobject_cast<LWVertexGroup*>(newitem);

    if (coordgui != nullptr) {
        coordgui->enableCoordinateSelection(mainWindow);
    }

    if (entitygui != nullptr) {
        entitygui->enableWidgetSelection(mainWindow);
    }

    if (lwVertexGroup != nullptr) {
        lwVertexGroup->setMainWindow(mainWindow);
    }

    QListWidgetItem* itemW = new QListWidgetItem();
    itemW->setSizeHint(newitem->sizeHint());
    listWidget->addItem(itemW);
    listWidget->setItemWidget(itemW, newitem);
    itemList.push_back(newitem);
    _addedKeys.insert(key);
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

void ListGUI::plusButtonClicked() {
    if (_listType == ListType::COORDINATE) {
        std::string newkey = _key + "_coord" + std::to_string(itemIdCount);
        CoordinateGUI* coordWidget = new CoordinateGUI("Coordinate " + std::to_string(itemIdCount));
        itemIdCount++;
        addItem(newkey, coordWidget);

        for (kaguya::LuaRef& cb : _callbacks) {
            coordWidget->addFinishCallback(cb);
            cb();
        }
    }

    if (_listType == ListType::LW_VERTEX) {
        std::string newkey = _key + "_lwvertex" + std::to_string(itemIdCount);
        LWVertexGroup* lwVertexGroup = new LWVertexGroup("LWVertex Group " + std::to_string(itemIdCount));
        itemIdCount++;
        addItem(newkey, lwVertexGroup);

        for (kaguya::LuaRef& cb : _callbacks) {
            lwVertexGroup->addCallback(cb);
            cb();
        }
    }
}

void ListGUI::minusButtonClicked() {
    if (_listType == ListType::COORDINATE) {
        QListWidgetItem* selectedItem = listWidget->currentItem();
        CoordinateGUI* coordgui = qobject_cast<CoordinateGUI*>(listWidget->itemWidget(selectedItem));
        
        _addedKeys.erase(_addedKeys.find(coordgui->key()));
        for (std::vector<InputGUI*>::iterator iter = itemList.begin(); iter != itemList.end();++iter) {
            if ((*iter)->key() == coordgui->key()) {
                itemList.erase(iter);
                break;
            }
        }
        selectedItem->setHidden(true);

        for (kaguya::LuaRef& cb : _callbacks) {
            cb();
        }
    }

    listWidget->setCurrentItem(listWidget->item(0));
}

void ListGUI::setListType(ListGUI::ListType listTypeIn) {
    if (listTypeIn == ListType::COORDINATE) {
        // check if all already added widgets are of type coordinate
        for (int i = 0; i < itemList.size(); i++) {
            CoordinateGUI* coordgui = qobject_cast<CoordinateGUI*>(itemList[i]);
            if (coordgui == nullptr) {
                return;
            }
        }

        _listType = listTypeIn;
        qobject_cast<QPushButton*>(ui->verticalLayout->itemAt(2)->layout()->itemAt(0)->widget())->setEnabled(true);
        qobject_cast<QPushButton*>(ui->verticalLayout->itemAt(2)->layout()->itemAt(1)->widget())->setEnabled(true);
    }

    if (listTypeIn == ListType::LW_VERTEX) {
        // check if all already added widgets are of type coordinate
        for (int i = 0; i < itemList.size(); i++) {
            LWVertexGroup* lwvertex = qobject_cast<LWVertexGroup*>(itemList[i]);
            if (lwvertex == nullptr) {
                return;
            }
        }

        _listType = listTypeIn;
        qobject_cast<QPushButton*>(ui->verticalLayout->itemAt(2)->layout()->itemAt(0)->widget())->setEnabled(true);
        qobject_cast<QPushButton*>(ui->verticalLayout->itemAt(2)->layout()->itemAt(1)->widget())->setEnabled(true);
    }
}

void ListGUI::setListType(const std::string& listTypeStr) {
    if (listTypeStr == "coordinate") {
        setListType(ListType::COORDINATE);
    }

    if (listTypeStr == "lwvertex") {
        setListType(ListType::LW_VERTEX);
    }
}

ListGUI::ListType ListGUI::listType() const {
    return _listType;
}

void ListGUI::setValue(std::vector<lc::geo::Coordinate> coords) {
    for (lc::geo::Coordinate& coord : coords) {
        std::string newkey = _key + "_coord" + std::to_string(itemIdCount);
        CoordinateGUI* coordinateGUI = new CoordinateGUI("Coordinate " + std::to_string(itemIdCount));
        itemIdCount++;
        coordinateGUI->setValue(coord);
        addItem(newkey, coordinateGUI);
    }
}

void ListGUI::addCallbackToAll(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
    if (_listType == ListType::COORDINATE) {
        for (InputGUI* inputWidget : itemList) {
            CoordinateGUI* coordWidget = qobject_cast<CoordinateGUI*>(inputWidget);
            coordWidget->addFinishCallback(cb);
        }
    }

    if (_listType == ListType::LW_VERTEX) {
        for (InputGUI* inputWidget : itemList) {
            LWVertexGroup* lwVertex = qobject_cast<LWVertexGroup*>(inputWidget);
            lwVertex->addCallback(cb);
        }
    }
}

void ListGUI::guiItemChanged(QListWidgetItem* current, QListWidgetItem* previous) {
    // if called by outside (property editor)
    if (current == nullptr) {
        current = listWidget->currentItem();
    }

    if (_listType == ListType::COORDINATE) {
        CoordinateGUI* coordgui = qobject_cast<CoordinateGUI*>(listWidget->itemWidget(current));
        lc::geo::Coordinate coord = coordgui->value();
        lc::viewer::drawable::TempEntities_SPtr tempEntities = mainWindow->cadMdiChild()->tempEntities();

        if (_selectedCoordinate != nullptr) {
            tempEntities->removeEntity(_selectedCoordinate);
        }

        auto layer = std::make_shared<lc::meta::Layer>("highlightGUI", lc::meta::MetaLineWidthByValue(4), lc::Color(254, 254, 1));
        _selectedCoordinate = std::make_shared<const lc::entity::Circle>(coord, 10, layer);
        tempEntities->addEntity(_selectedCoordinate);
    }
}
