#include "propertyeditor.h"

#include "widgets/guiAPI/entitypickervisitor.h"
#include <QVBoxLayout>

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

}

void PropertyEditor::addEntity(lc::entity::CADEntity_CSPtr entity) {
    api::EntityPickerVisitor entityVisitor;
    entity->dispatch(entityVisitor);
    std::cout << "Entity Selected :- " << entityVisitor.getEntityInformation() << std::endl;
}
