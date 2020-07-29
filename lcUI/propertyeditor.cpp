#include "propertyeditor.h"

#include "widgets/guiAPI/entitypickervisitor.h"
#include <QVBoxLayout>
#include <QScrollArea>

#include "widgets/guiAPI/numbergui.h"
#include "widgets/guiAPI/coordinategui.h"
#include "widgets/guiAPI/anglegui.h"
#include "widgets/guiAPI/textgui.h"
#include "widgets/guiAPI/listgui.h"
#include "widgets/guiAPI/horizontalgroupgui.h"

#include <cad/builders/lwpolyline.h>
#include "widgets/widgettitlebar.h"

using namespace lc::ui;

PropertyEditor* PropertyEditor::instance = nullptr;

PropertyEditor::PropertyEditor(lc::ui::MainWindow* mainWindow)
    :
    QDockWidget("Property Editor", mainWindow),
    InputGUIContainer("property_editor", mainWindow)
{
    QScrollArea* parentWidget = new QScrollArea(this);

    parentWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    parentWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    parentWidget->setWidgetResizable(true);

    QTreeWidget* widget = new QTreeWidget(parentWidget);
    widget->setColumnCount(2);
    widget->setHeaderHidden(true);
    widget->setObjectName("guiContainer");

    widget->setStyleSheet("QTreeWidget::item{ background: rgb(252, 255, 247); border : 1px solid rgb(156, 220, 31); margin-left: 2; padding-left: 5px;}");
    widget->setColumnWidth(0, 200);

    parentWidget->setWidget(widget);
    this->setWidget(parentWidget);

    widgets::WidgetTitleBar* titleBar = new widgets::WidgetTitleBar("Property Editor", this,
        widgets::WidgetTitleBar::TitleBarOptions::HorizontalOnHidden);
    this->setTitleBarWidget(titleBar);
}

PropertyEditor* PropertyEditor::GetPropertyEditor(lc::ui::MainWindow* mainWindow){
    if (instance == nullptr) {
        instance = new PropertyEditor(mainWindow);
    }

    return instance;
}

void PropertyEditor::clear(std::vector<lc::entity::CADEntity_CSPtr> selectedEntities) {
    QTreeWidget* guicontainer = this->widget()->findChild<QTreeWidget*>("guiContainer");

    for (std::set<unsigned long>::iterator iter = _selectedEntities.begin(); iter != _selectedEntities.end();) {
        bool found = false;

        for (lc::entity::CADEntity_CSPtr cadEnt : selectedEntities) {
            if (cadEnt->id() == *iter) {
                found = true;
                break;
            }
        }

        if (!found) {
            for (const std::string& keyStr : _entityProperties[*iter]) {
                removeInputGUI(keyStr, false);
            }

            guicontainer->removeItemWidget(_entityGroup[*iter], 0);
            guicontainer->takeTopLevelItem(guicontainer->indexOfTopLevelItem(_entityGroup[*iter]));
            delete _entityGroup[*iter];

            _entityProperties.erase(*iter);
            _entityGroup.erase(*iter);
            iter = _selectedEntities.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

void PropertyEditor::addEntity(lc::entity::CADEntity_CSPtr entity) {
    QTreeWidget* guicontainer = this->widget()->findChild<QTreeWidget*>("guiContainer");

    if (_selectedEntities.find(entity->id()) == _selectedEntities.end()) {
        _selectedEntities.insert(entity->id());
        _entityProperties[entity->id()] = std::vector<std::string>();

        // Get entity information
        api::EntityPickerVisitor entityVisitor;
        entity->dispatch(entityVisitor);
        QString entityInfo = QString(entityVisitor.getEntityInformation().c_str()) + QString(" #") + QString::number(entity->id());
        QTreeWidgetItem* treeItem = new QTreeWidgetItem();
        _entityGroup[entity->id()] = treeItem;
        guicontainer->addTopLevelItem(treeItem);
        guicontainer->setItemWidget(treeItem, 0, new QLabel(entityInfo));
    }

    createPropertiesWidgets(entity->id(), entity->availableProperties());
    createCustomWidgets(entity);
}

bool PropertyEditor::addWidget(const std::string& key, api::InputGUI* guiWidget) {
    if (_entityGroup.find(_currentEntity) == _entityGroup.end()) {
        return false;
    }
    QTreeWidget* guicontainer = this->widget()->findChild<QTreeWidget*>("guiContainer");

    bool success = InputGUIContainer::addWidget(key, guiWidget);
    if (success) {
        QTreeWidgetItem* entityChildItem = new QTreeWidgetItem();
        _entityGroup[_currentEntity]->addChild(entityChildItem);
        guicontainer->setItemWidget(entityChildItem, 0, new QLabel(guiWidget->label().c_str()));
        guiWidget->hideLabel();
        guicontainer->setItemWidget(entityChildItem, 1, guiWidget);
    }
    return success;
}

void PropertyEditor::propertyChanged(const std::string& key) {
    kaguya::LuaRef propertiesTable = generateInfo(mainWindow->luaInterface()->luaState());
    lc::entity::CADEntity_CSPtr entity = mainWindow->cadMdiChild()->storageManager()->entityByID(_widgetKeyToEntity[key]);

    std::shared_ptr<lc::operation::EntityBuilder> entityBuilder = std::make_shared<lc::operation::EntityBuilder>(mainWindow->cadMdiChild()->document());

    // delete old entity
    entityBuilder->appendEntity(entity);
    entityBuilder->appendOperation(std::make_shared<lc::operation::Push>());
    entityBuilder->appendOperation(std::make_shared<lc::operation::Remove>());
    entityBuilder->execute();

    auto lastUnderscore = key.find_last_of("_");
    auto secondLastUnderscore = key.find_last_of("_", lastUnderscore-1);

    std::string entityType = key.substr(lastUnderscore + 1);
    std::string propertyName = key.substr(secondLastUnderscore + 1, lastUnderscore - secondLastUnderscore - 1);

    lc::entity::PropertiesMap propertiesList;

    if (entityType == "angle") {
        propertiesList[propertyName] = lc::entity::AngleProperty(propertiesTable[key].get<double>());
    }

    if (entityType == "double") {
        propertiesList[propertyName] = propertiesTable[key].get<double>();
    }

    if (entityType == "bool") {
        propertiesList[propertyName] = propertiesTable[key].get<bool>();
    }

    if (entityType == "coordinate") {
        propertiesList[propertyName] = propertiesTable[key].get<lc::geo::Coordinate>();
    }

    if (entityType == "text") {
        propertiesList[propertyName] = propertiesTable[key].get<std::string>();
    }

    if (entityType == "vector") {
        propertiesList[propertyName] = propertiesTable[key].get<std::vector<lc::geo::Coordinate>>();
    }

    // returns nullptr if not custom property
    lc::entity::CADEntity_CSPtr changedEntity = customPropertyChanged(key, entityType, propertiesTable, entity);

    if (changedEntity == nullptr) {
        changedEntity = entity->setProperties(propertiesList);
    }

    // add new entity
    entityBuilder->appendEntity(changedEntity);
    entityBuilder->execute();

    // after entity has been changed
    if (entityType == "vector" || entityType == "customLWPolyline") {
        api::ListGUI* listgui = qobject_cast<api::ListGUI*>(_inputWidgets[key]);
        listgui->guiItemChanged(nullptr, nullptr);
    }
}

lc::entity::CADEntity_CSPtr PropertyEditor::customPropertyChanged(const std::string& key, const std::string& entityType, kaguya::LuaRef propertiesTable, lc::entity::CADEntity_CSPtr oldEntity) {
    if (entityType == "customLWPolyline") {
        kaguya::LuaTable entTable = propertiesTable[key];
        std::vector<kaguya::LuaRef> vertexKeys = entTable.keys();

        std::vector<lc::builder::LWBuilderVertex> builderVertices;
        for (kaguya::LuaRef vertexKey : vertexKeys)
        {
            std::string vertKey = vertexKey.get<std::string>();
            kaguya::LuaTable vertexTable = propertiesTable[vertexKey];
            std::vector<kaguya::LuaRef> vertexPropertiesKeys = vertexTable.keys();

            lc::geo::Coordinate loc;
            double sWidth;
            double eWidth;
            double bulge;

            for (kaguya::LuaRef vertexPropKey : vertexPropertiesKeys)
            {
                std::string propKey = vertexPropKey.get<std::string>();
                auto lastUnderscore = propKey.find_last_of("_");
                std::string propType = propKey.substr(lastUnderscore + 1);
                
                if (propType == "Location") {
                    loc = vertexTable[vertexPropKey].get<lc::geo::Coordinate>();
                }

                if (propType == "StartWidth") {
                    sWidth = vertexTable[vertexPropKey].get<double>();
                }

                if (propType == "EndWidth") {
                    eWidth = vertexTable[vertexPropKey].get<double>();
                }

                if (propType == "Bulge") {
                    bulge = vertexTable[vertexPropKey].get<double>();
                }
            }

            builderVertices.push_back(lc::builder::LWBuilderVertex(loc, sWidth, eWidth, bulge, 0));
        }

        lc::builder::LWPolylineBuilder lwPolylineBuilder;
        lwPolylineBuilder.copy(std::dynamic_pointer_cast<const lc::entity::LWPolyline>(oldEntity));
        lwPolylineBuilder.setVertices(builderVertices);

        return lwPolylineBuilder.build();
    }

    return nullptr;
}

void PropertyEditor::createPropertiesWidgets(unsigned long entityID, const lc::entity::PropertiesMap& entityProperties) {
    _currentEntity = entityID;
    kaguya::State state(mainWindow->luaInterface()->luaState());

    for (auto iter = entityProperties.cbegin(); iter != entityProperties.cend(); ++iter) {
        std::string key = generatePropertyKey(entityID, iter->first, iter->second.which());

        if (_addedKeys.find(key) == _addedKeys.end()) {
            // angleproperty
            if (iter->second.which() == 0) {
                lc::ui::api::AngleGUI* anglegui = new lc::ui::api::AngleGUI(std::string(1, std::toupper(iter->first[0])) + iter->first.substr(1));
                anglegui->setValue(boost::get<lc::entity::AngleProperty>(iter->second).Get());
                state.dostring("anglePropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('" + key + "') end");
                anglegui->addFinishCallback(state["anglePropertyCalled"]);
                addWidget(key, anglegui);
                state["anglePropertyCalled"] = nullptr;
            }

            // double
            if (iter->second.which() == 1) {
                lc::ui::api::NumberGUI* numbergui = new lc::ui::api::NumberGUI(std::string(1,std::toupper(iter->first[0])) + iter->first.substr(1));
                numbergui->setValue(boost::get<double>(iter->second));
                state.dostring("numberPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('" + key + "') end");
                numbergui->addCallback(state["numberPropertyCalled"]);
                addWidget(key, numbergui);
                state["numberPropertyCalled"] = nullptr;
            }

            // bool
            if (iter->second.which() == 2) {
                lc::ui::api::CheckBoxGUI* checkboxgui = new lc::ui::api::CheckBoxGUI(std::string(1, std::toupper(iter->first[0])) + iter->first.substr(1));
                checkboxgui->setValue(boost::get<bool>(iter->second));
                state.dostring("boolPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('" + key + "') end");
                checkboxgui->addCallback(state["boolPropertyCalled"]);
                addWidget(key, checkboxgui);
                state["boolPropertyCalled"] = nullptr;
            }

            // coordinate
            if (iter->second.which() == 3) {
                lc::ui::api::CoordinateGUI* coordinategui = new lc::ui::api::CoordinateGUI(std::string(1, std::toupper(iter->first[0])) + iter->first.substr(1));
                coordinategui->setValue(boost::get<lc::geo::Coordinate>(iter->second));
                state.dostring("coordinatePropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('" + key + "') end");
                coordinategui->addFinishCallback(state["coordinatePropertyCalled"]);
                addWidget(key, coordinategui);
                state["coordinatePropertyCalled"] = nullptr;
            }

            // text (string)
            if (iter->second.which() == 4) {
                lc::ui::api::TextGUI* textgui = new lc::ui::api::TextGUI(std::string(1, std::toupper(iter->first[0])) + iter->first.substr(1));
                textgui->setValue(boost::get<std::string>(iter->second));
                state.dostring("textPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('" + key + "') end");
                textgui->addFinishCallback(state["textPropertyCalled"]);
                addWidget(key, textgui);
                state["textPropertyCalled"] = nullptr;
            }
            
            // vector
            if (iter->second.which() == 5) {
                lc::ui::api::ListGUI* listgui = new lc::ui::api::ListGUI(std::string(1, std::toupper(iter->first[0])) + iter->first.substr(1), lc::ui::api::ListGUI::ListType::COORDINATE);
                listgui->setMainWindow(mainWindow);

                std::vector<lc::geo::Coordinate> coords = boost::get<std::vector<lc::geo::Coordinate>>(iter->second);
                listgui->setValue(coords);

                state.dostring("vectorPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('" + key + "') end");
                listgui->addCallbackToAll(state["vectorPropertyCalled"]);
                addWidget(key, listgui);
                state["vectorPropertyCalled"] = nullptr;
            }

            _entityProperties[entityID].push_back(key);
            _widgetKeyToEntity[key] = entityID;
        }
    }
}

void PropertyEditor::createCustomWidgets(lc::entity::CADEntity_CSPtr entity) {
    _currentEntity = entity->id();
    kaguya::State state(mainWindow->luaInterface()->luaState());

    api::EntityPickerVisitor entityVisitor;
    entity->dispatch(entityVisitor);

    if (entityVisitor.getEntityInformation() == "LWPolyline") {
        std::string key = "entity" + std::to_string(entity->id()) + "_" + "customLWPolyline";
        lc::ui::api::ListGUI* listgui = new lc::ui::api::ListGUI("LWVertex List", lc::ui::api::ListGUI::ListType::LW_VERTEX);
        listgui->setMainWindow(mainWindow);
        
        lc::builder::LWPolylineBuilder lwPolylineBuilder;
        lwPolylineBuilder.copy(std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity));
        listgui->setValue(lwPolylineBuilder.getVertices());

        state.dostring("customPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('" + key + "') end");
        listgui->addCallbackToAll(state["customPropertyCalled"]);
        addWidget(key, listgui);
        state["customPropertyCalled"] = nullptr;

        _entityProperties[entity->id()].push_back(key);
        _widgetKeyToEntity[key] = entity->id();
    }
}

std::string PropertyEditor::generatePropertyKey(unsigned long entityID, const std::string& propKey, int propType) const {
    std::string key = "entity" + std::to_string(entityID) + "_" + propKey;

    switch (propType)
    {
    case 0:
        key += "_angle";
        break;
    case 1:
        key += "_double";
        break;
    case 2:
        key += "_bool";
        break;
    case 3:
        key += "_coordinate";
        break;
    case 4:
        key += "_text";
        break;
    case 5:
        key += "_vector";
        break;
    }

    return key;
}

void PropertyEditor::closeEvent(QCloseEvent* event)
{
    this->widget()->hide();
    event->ignore();
}
