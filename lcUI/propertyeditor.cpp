#include "propertyeditor.h"

using namespace lc::ui;

PropertyEditor* PropertyEditor::instance = 0;

PropertyEditor::PropertyEditor()
{

}

PropertyEditor* PropertyEditor::GetPropertyEditor()
{
    if (instance == nullptr) {
        instance = new PropertyEditor();
    }

    return instance;
}
