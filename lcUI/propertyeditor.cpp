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

    if (entityVisitor.getEntityInformation() == "Circle") {
        if (_addedKeys.find("circle_radius") == _addedKeys.end()) {
            lc::ui::api::NumberGUI* radiusgui = new lc::ui::api::NumberGUI("Radius");
            state.dostring("radiusPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('circle_radius') end");
            radiusgui->addCallback(state["radiusPropertyCalled"]);
            addWidget("circle_radius", radiusgui);
            state["radiusPropertyCalled"] = nullptr;
            _selectedEntity[entity->id()].push_back("circle_radius");
        }

        if (_addedKeys.find("circle_center") == _addedKeys.end()) {
            lc::ui::api::CoordinateGUI* coordgui = new lc::ui::api::CoordinateGUI("Center");
            state.dostring("coordPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('circle_center') end");
            coordgui->addOnChangeCallback(state["coordPropertyCalled"]);
            addWidget("circle_center", coordgui);
            state["coordPropertyCalled"] = nullptr;
            _selectedEntity[entity->id()].push_back("circle_center");
        }
    }
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
}
