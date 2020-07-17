#include "propertyeditor.h"

#include "widgets/guiAPI/entitypickervisitor.h"
#include <QVBoxLayout>

#include "widgets/guiAPI/numbergui.h"
#include "widgets/guiAPI/coordinategui.h"

using namespace lc::ui;

PropertyEditor* PropertyEditor::instance = 0;

PropertyEditor::PropertyEditor(lc::ui::MainWindow* mainWindow)
    :
    QDockWidget("Property Editor", mainWindow),
    InputGUIContainer("property_editor", mainWindow)
{
    QWidget* parentWidget = new QWidget(this);
    this->setWidget(parentWidget);
    QVBoxLayout* verticalLayout = new QVBoxLayout();
    parentWidget->setLayout(verticalLayout);
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
}

void PropertyEditor::addEntity(lc::entity::CADEntity_CSPtr entity) {
    api::EntityPickerVisitor entityVisitor;
    entity->dispatch(entityVisitor);
    mainWindow->cliCommand()->write("Entity Selected : -" + entityVisitor.getEntityInformation());
    kaguya::State state(mainWindow->luaInterface()->luaState());

    if (_selectedEntity.find(entity->id()) == _selectedEntity.end()) {
        _selectedEntity[entity->id()] = std::vector<std::string>();
    }

    createPropertiesWidgets(entity->id(), entity->availableProperties());
}

bool PropertyEditor::addWidget(const std::string& key, api::InputGUI* guiWidget) {
    bool success = InputGUIContainer::addWidget(key, guiWidget);
    if (success) {
        guiWidget->setParent(this);
        this->widget()->layout()->addWidget(guiWidget);
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
    mainWindow->cliCommand()->write("PROPERTY CHANGES - " + key);
    kaguya::LuaRef propertiesTable = generateInfo(mainWindow->luaInterface()->luaState());
    lc::entity::CADEntity_CSPtr entity = mainWindow->cadMdiChild()->storageManager()->entityByID(_widgetKeyToEntity[key]);

    std::shared_ptr<lc::operation::EntityBuilder> entityBuilder = std::make_shared<lc::operation::EntityBuilder>(mainWindow->cadMdiChild()->document());

    entityBuilder->appendEntity(entity);
    entityBuilder->appendOperation(std::make_shared<lc::operation::Push>());
    entityBuilder->appendOperation(std::make_shared<lc::operation::Remove>());
    entityBuilder->execute();

    propertyVisitor.setPropertyKey(key);
    propertyVisitor.setPropertyTable(propertiesTable);
    propertyVisitor.setEntityBuilder(entityBuilder);
    entity->dispatch(propertyVisitor);
}

void PropertyEditor::createPropertiesWidgets(unsigned long entityID, const lc::entity::CADEntity::PropertiesMap& entityProperties) {
    kaguya::State state(mainWindow->luaInterface()->luaState());

    for (auto iter = entityProperties.cbegin(); iter != entityProperties.cend(); ++iter) {
        const std::string key = "entity" + std::to_string(entityID) + "_" + iter->first;
        std::cout << "Key is " << key << std::endl;

        if (_addedKeys.find(key) == _addedKeys.end()) {
            // double
            if (iter->second.which() == 0) {
                lc::ui::api::NumberGUI* numbergui = new lc::ui::api::NumberGUI(iter->first);
                numbergui->setValue(boost::get<double>(iter->second));
                state.dostring("numberPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('" + key + "') end");
                numbergui->addCallback(state["numberPropertyCalled"]);
                addWidget(key, numbergui);
                state["numberPropertyCalled"] = nullptr;
            }

            // bool
            if (iter->second.which() == 1) {
            }

            // coordinate
            if (iter->second.which() == 2) {
                lc::ui::api::CoordinateGUI* coordinategui = new lc::ui::api::CoordinateGUI(iter->first);
                coordinategui->setValue(boost::get<lc::geo::Coordinate>(iter->second));
                state.dostring("coordinatePropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('" + key + "') end");
                coordinategui->addFinishCallback(state["coordinatePropertyCalled"]);
                addWidget(key, coordinategui);
                state["coordinatePropertyCalled"] = nullptr;
            }

            _selectedEntity[entityID].push_back(key);
            _widgetKeyToEntity[key] = entityID;
        }
    }
}
