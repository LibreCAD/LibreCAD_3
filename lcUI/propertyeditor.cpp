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
        lc::entity::Circle_CSPtr circleEntity = std::dynamic_pointer_cast<const lc::entity::Circle>(entity);

        if (_addedKeys.find("circle_radius") == _addedKeys.end()) {
            lc::ui::api::NumberGUI* radiusgui = new lc::ui::api::NumberGUI("Radius");
            radiusgui->setValue(circleEntity->radius());
            state.dostring("radiusPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('circle_radius') end");
            radiusgui->addCallback(state["radiusPropertyCalled"]);
            addWidget("circle_radius", radiusgui);
            state["radiusPropertyCalled"] = nullptr;
            _selectedEntity[entity->id()].push_back("circle_radius");
            _widgetKeyToEntity["circle_radius"] = entity->id();
        }

        if (_addedKeys.find("circle_center") == _addedKeys.end()) {
            lc::ui::api::CoordinateGUI* coordgui = new lc::ui::api::CoordinateGUI("Center");
            coordgui->setValue(circleEntity->center());
            state.dostring("coordPropertyCalled = function() lc.PropertyEditor:GetPropertyEditor():propertyChanged('circle_center') end");
            coordgui->addFinishCallback(state["coordPropertyCalled"]);
            addWidget("circle_center", coordgui);
            state["coordPropertyCalled"] = nullptr;
            _selectedEntity[entity->id()].push_back("circle_center");
            _widgetKeyToEntity["circle_center"] = entity->id();
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

    if (key == "circle_radius") {
        double circleRadius = propertiesTable[key].get<double>();
        std::cout << "Value for " << key << " - " << circleRadius << std::endl;


        /*lc::entity::Circle_CSPtr oldEntity = std::dynamic_pointer_cast<const lc::entity::Circle>(mainWindow->cadMdiChild()->storageManager()->entityByID(_widgetKeyToEntity[key]));

        lc::builder::CircleBuilder circleBuilder;
        circleBuilder.copy(oldEntity);
        circleBuilder.setRadius(circleRadius);*/

        /*lc::entity::Circle_CSPtr oldEntity = std::dynamic_pointer_cast<const lc::entity::Circle>(mainWindow->cadMdiChild()->storageManager()->entityByID(_widgetKeyToEntity[key]));
        lc::entity::Circle_SPtr changedEntity = std::make_shared<lc::entity::Circle>(
            oldEntity->center(),
            circleRadius,
            oldEntity->layer(),
            oldEntity->metaInfo(),
            oldEntity->block());
        changedEntity->setID(oldEntity->id());

        std::shared_ptr<lc::operation::EntityBuilder> entityBuilder = std::make_shared<lc::operation::EntityBuilder>(mainWindow->cadMdiChild()->document());
        entityBuilder->appendEntity(oldEntity);
        entityBuilder->appendOperation(std::make_shared<lc::operation::Push>());
        entityBuilder->appendOperation(std::make_shared<lc::operation::Remove>());
        entityBuilder->execute();

        entityBuilder->appendEntity(changedEntity);
        entityBuilder->execute();*/
    }

    if (key == "circle_center") {
        lc::geo::Coordinate circleCenter = propertiesTable[key].get<lc::geo::Coordinate>();
        std::cout << "Value for " << key << " - " << circleCenter << std::endl;

        /*lc::entity::Circle_CSPtr oldEntity = std::dynamic_pointer_cast<const lc::entity::Circle>(mainWindow->cadMdiChild()->storageManager()->entityByID(_widgetKeyToEntity[key]));
        lc::entity::Circle_SPtr changedEntity = std::make_shared<lc::entity::Circle>(
            circleCenter,
            oldEntity->radius(),
            oldEntity->layer(),
            oldEntity->metaInfo(),
            oldEntity->block());
        changedEntity->setID(oldEntity->id());

        std::shared_ptr<lc::operation::EntityBuilder> entityBuilder = std::make_shared<lc::operation::EntityBuilder>(mainWindow->cadMdiChild()->document());
        entityBuilder->appendEntity(oldEntity);
        entityBuilder->appendOperation(std::make_shared<lc::operation::Push>());
        entityBuilder->appendOperation(std::make_shared<lc::operation::Remove>());
        entityBuilder->execute();

        entityBuilder->appendEntity(changedEntity);
        entityBuilder->execute();*/
    }
}
