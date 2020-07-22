#include "propertyeditor.h"

#include "widgets/guiAPI/entitypickervisitor.h"
#include <QVBoxLayout>
#include <QScrollArea>

#include "widgets/guiAPI/numbergui.h"
#include "widgets/guiAPI/coordinategui.h"
#include "widgets/guiAPI/anglegui.h"
#include "widgets/guiAPI/textgui.h"
#include "widgets/guiAPI/listgui.h"

using namespace lc::ui;

PropertyEditor* PropertyEditor::instance = 0;

PropertyEditor::PropertyEditor(lc::ui::MainWindow* mainWindow)
    :
    QDockWidget("Property Editor", mainWindow),
    InputGUIContainer("property_editor", mainWindow)
{
    QScrollArea* parentWidget = new QScrollArea(this);

    parentWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    parentWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    parentWidget->setWidgetResizable(true);
    QWidget* widget = new QWidget(parentWidget);
    widget->setObjectName("guiContainer");
    parentWidget->setWidget(widget);
    this->setWidget(parentWidget);
    QVBoxLayout* verticalLayout = new QVBoxLayout();
    widget->setLayout(verticalLayout);
}

PropertyEditor* PropertyEditor::GetPropertyEditor(lc::ui::MainWindow* mainWindow){
    if (instance == nullptr) {
        instance = new PropertyEditor(mainWindow);
    }

    return instance;
}

void PropertyEditor::clear(std::vector<lc::entity::CADEntity_CSPtr> selectedEntities) {
    std::vector<std::string> inputGUIsToRemove;
    
    for (std::map<unsigned long, std::vector<std::string>>::iterator iter = _selectedEntity.begin(); iter != _selectedEntity.end(); ++iter) {
        bool found = false;

        for (lc::entity::CADEntity_CSPtr cadEnt : selectedEntities) {
            if (cadEnt->id() == iter->first) {
                found = true;
                break;
            }
        }

        if (!found) {
            for (const std::string& keyStr : iter->second) {
                removeInputGUI(keyStr);
            }

            iter = _selectedEntity.erase(iter);
        }
    }

    for (std::map<unsigned long, QGroupBox*>::iterator iter = _entityGroup.begin(); iter != _entityGroup.end(); ++iter) {
        bool found = false;

        for (lc::entity::CADEntity_CSPtr cadEnt : selectedEntities) {
            if (cadEnt->id() == iter->first) {
                found = true;
                break;
            }
        }

        if (!found) {
            delete _entityGroup[iter->first];
            iter = _entityGroup.erase(iter);
        }
    }
}

void PropertyEditor::addEntity(lc::entity::CADEntity_CSPtr entity) {
    if (_selectedEntity.find(entity->id()) == _selectedEntity.end()) {
        _selectedEntity[entity->id()] = std::vector<std::string>();
    }

    if (_entityGroup.find(entity->id()) == _entityGroup.end()) {
        // Get entity information
        api::EntityPickerVisitor entityVisitor;
        entity->dispatch(entityVisitor);
        QString entityInfo = QString(entityVisitor.getEntityInformation().c_str()) + QString(" #") + QString::number(entity->id());

        // Create and add entity group
        QGroupBox* entityGroup = new QGroupBox(entityInfo);
        _entityGroup[entity->id()] = entityGroup;
        QVBoxLayout* entityLayout = new QVBoxLayout();
        entityGroup->setLayout(entityLayout);

        QWidget* guicontainer = this->widget()->findChild<QWidget*>("guiContainer");
        guicontainer->layout()->addWidget(entityGroup);
    }

    createPropertiesWidgets(entity->id(), entity->availableProperties());
}

bool PropertyEditor::addWidget(const std::string& key, api::InputGUI* guiWidget) {
    if (_entityGroup.find(_currentEntity) == _entityGroup.end()) {
        std::cout << "Entity group does not exist" << std::endl;
        return false;
    }
    QGroupBox* entityGroup = _entityGroup[_currentEntity];

    bool success = InputGUIContainer::addWidget(key, guiWidget);
    if (success) {
        entityGroup->layout()->addWidget(guiWidget);
    }
    return success;
}

bool PropertyEditor::addWidget(const std::string& key, api::ButtonGUI* buttonWidget) {
    return InputGUIContainer::addWidget(key, buttonWidget);
}

bool PropertyEditor::addWidget(const std::string& key, api::CheckBoxGUI* checkboxWidget) {
    return InputGUIContainer::addWidget(key, checkboxWidget);
}

void PropertyEditor::propertyChanged(const std::string& key) {
    kaguya::LuaRef propertiesTable = generateInfo(mainWindow->luaInterface()->luaState());
    lc::entity::CADEntity_CSPtr entity = mainWindow->cadMdiChild()->storageManager()->entityByID(_widgetKeyToEntity[key]);

    std::shared_ptr<lc::operation::EntityBuilder> entityBuilder = std::make_shared<lc::operation::EntityBuilder>(mainWindow->cadMdiChild()->document());

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

    lc::entity::CADEntity_CSPtr changedEntity = entity->setProperties(propertiesList);

    entityBuilder->appendEntity(changedEntity);
    entityBuilder->execute();

    // after entity has been changed
    if (entityType == "vector") {
        api::ListGUI* listgui = qobject_cast<api::ListGUI*>(_inputWidgets[key]);
        listgui->guiItemChanged(nullptr, nullptr);
    }
}

void PropertyEditor::createPropertiesWidgets(unsigned long entityID, const lc::entity::PropertiesMap& entityProperties) {
    _currentEntity = entityID;
    kaguya::State state(mainWindow->luaInterface()->luaState());

    for (auto iter = entityProperties.cbegin(); iter != entityProperties.cend(); ++iter) {
        std::string key = "entity" + std::to_string(entityID) + "_" + iter->first;
        if (iter->second.which() == 0) {
            key += "_angle";
        }

        // double
        if (iter->second.which() == 1) {
            key += "_double";
        }

        // bool
        if (iter->second.which() == 2) {
            key += "_bool";
        }

        // coordinate
        if (iter->second.which() == 3) {
            key += "_coordinate";
        }

        // text (string)
        if (iter->second.which() == 4) {
            key += "_text";
        }

        // vector
        if (iter->second.which() == 5) {
            key += "_vector";
        }

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

            _selectedEntity[entityID].push_back(key);
            _widgetKeyToEntity[key] = entityID;
        }
    }
}
