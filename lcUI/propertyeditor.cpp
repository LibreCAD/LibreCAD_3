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

PropertyEditor* PropertyEditor::GetPropertyEditor(lc::ui::MainWindow* mainWindow)
{
    if (instance == nullptr) {
        instance = new PropertyEditor(mainWindow);
    }

    return instance;
}

void PropertyEditor::clear() {
    for (api::InputGUI* widget : _inputWidgets) {
        delete widget;
    }

    _inputWidgets.clear();
    _addedKeys.clear();
    _selectedEntities.clear();
}

void PropertyEditor::addEntity(lc::entity::CADEntity_CSPtr entity) {
    api::EntityPickerVisitor entityVisitor;
    entity->dispatch(entityVisitor);
    std::cout << "Entity Selected :- " << entityVisitor.getEntityInformation() << std::endl;
    kaguya::State state(mainWindow->luaInterface()->luaState());
    _selectedEntities.insert(entity);

    if (entityVisitor.getEntityInformation() == "Circle") {
        if (_addedKeys.find("circle_radius") == _addedKeys.end()) {
            lc::ui::api::NumberGUI* radiusgui = new lc::ui::api::NumberGUI("Radius");
            state.dostring("radiusPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('circle_radius') end");
            radiusgui->addCallback(state["radiusPropertyCalled"]);
            addWidget("circle_radius", radiusgui);
            state["radiusPropertyCalled"] = nullptr;
        }

        if (_addedKeys.find("circle_center") == _addedKeys.end()) {
            lc::ui::api::CoordinateGUI* coordgui = new lc::ui::api::CoordinateGUI("Center");
            //state.dostring("coordPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('circle_center') end");
            //coordgui->addOnChangeCallback(state["coordPropertyCalled"]);
            addWidget("circle_center", coordgui);
            state["coordPropertyCalled"] = nullptr;
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
    mainWindow->cliCommand()->write("WOOOOHOOOO PROPERTY CHANGES - " + key);
}

bool PropertyEditor::containsEntity(lc::entity::CADEntity_CSPtr entity) {
    for (lc::entity::CADEntity_CSPtr curEntity : _selectedEntities) {
        if (*curEntity == *entity) {
            return true;
        }
    }

    return false;
}
