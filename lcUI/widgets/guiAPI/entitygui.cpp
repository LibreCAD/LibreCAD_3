#include "entitygui.h"
#include "ui_entitygui.h"

#include "cadmdichild.h"

using namespace lc::ui::api;

EntityGUI::EntityGUI(std::string label, QWidget* parent)
    :
    InputGUI(label, parent),
    mainWindow(nullptr),
    _entitySelectionEnabled(false),
    ui(new Ui::EntityGUI)
{
    ui->setupUi(this);

    _textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    _entityList = qobject_cast<QListWidget*>(ui->horizontalLayout->itemAt(1)->widget());
    _addButton = qobject_cast<QPushButton*>(ui->horizontalLayout->itemAt(2)->widget());
    ui->horizontalLayout->insertStretch(1);

    _textLabel->setText(QString(this->label().c_str()));

    connect(_addButton, &QPushButton::toggled, this, &EntityGUI::enableSelections);
}

EntityGUI::~EntityGUI()
{
    delete ui;
}

void EntityGUI::getLuaValue(kaguya::LuaRef& table) {
    if (_selectedEntitiesList.size() == 1) {
        table[_key] = _selectedEntitiesList[0];
    }
    else {
        table[_key] = value();
    }
}

void EntityGUI::enableWidgetSelection(lc::ui::MainWindow* mainWindowIn) {
    if (mainWindow != nullptr) {
        return;
    }

    this->mainWindow = mainWindowIn;
    if (mainWindow != nullptr) {
        lc::ui::CadMdiChild* cadMdiChild = mainWindow->cadMdiChild();
        connect(cadMdiChild->viewerProxy(), &LCADViewerProxy::mouseReleaseEvent, this, &EntityGUI::entitySelected);
    }
}

void EntityGUI::enableSelections(bool toggle) {
    _entitySelectionEnabled = toggle;

    if (toggle) {
        mainWindow->selectNone();
        mainWindow->activateWindow();
    }
}

void EntityGUI::entitySelected() {
    if (_entitySelectionEnabled) {
        lc::ui::CadMdiChild* cadMdiChild = mainWindow->cadMdiChild();
        std::vector<lc::entity::CADEntity_CSPtr> selectedEntities = cadMdiChild->selection();
        
        if (selectedEntities.size() == 0) {
            return;
        }

        for (lc::entity::CADEntity_CSPtr selectedEntity : selectedEntities) {
            addEntity(selectedEntity);
        }

        _entitySelectionEnabled = false;
        parentWidget()->activateWindow();
        this->setFocus();
        _addButton->toggled(false);
        _addButton->setChecked(false);
    }
}

void EntityGUI::addEntity(lc::entity::CADEntity_CSPtr entity) {
    if (_addedEntities.find(entity->id()) != _addedEntities.end()) {
        return;
    }

    QString entityInfo = getEntityInformation(entity);

    QListWidgetItem* item = new QListWidgetItem(entityInfo);
    item->setData(Qt::UserRole, _selectedEntitiesList.size());
    _selectedEntitiesList.push_back(entity);
    _entityList->addItem(item);
    _addedEntities.insert(entity->id());
}

QString EntityGUI::getEntityInformation(lc::entity::CADEntity_CSPtr entity) {
    int numId = entity->id();
    return QString("Entity" + QString::number(numId));
}

std::vector<lc::entity::CADEntity_CSPtr> EntityGUI::value() const {
    return _selectedEntitiesList;
}

void EntityGUI::setValue(std::vector<lc::entity::CADEntity_CSPtr> newSelectedEntities) {
    for (lc::entity::CADEntity_CSPtr entity : newSelectedEntities) {
        addEntity(entity);
    }
}
