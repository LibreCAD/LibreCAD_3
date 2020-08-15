#include "toolbargroup.h"

#include <QGridLayout>

using namespace lc::ui::api;

ToolbarGroup::ToolbarGroup(const char* groupName, int width, QWidget* parent)
    :
    QGroupBox(groupName, parent),
    _width(width),
    _nonButtonGroup(false),
    _count(0)
{
    this->setLayout(new QGridLayout());
}

void ToolbarGroup::addButton(ToolbarButton* button) {
    auto gridLayout = qobject_cast<QGridLayout*>(this->layout());
    gridLayout->addWidget(button, _count / _width, _count % _width, 1, 1);

    QObject::connect(button, SIGNAL(removeButton(ToolbarButton*)), this, SLOT(removeButton(ToolbarButton*)));

    _count++;
}

void ToolbarGroup::setWidth(int width) {
    std::vector<QWidget*> widgetsList;
    auto gridLayout = qobject_cast<QGridLayout*>(this->layout());

    for (int i = 0; i < gridLayout->count(); i++) {
        QWidget* currentWidget = gridLayout->itemAt(i)->widget();
        widgetsList.push_back(currentWidget);
    }

    for (QWidget* currentWidget : widgetsList)
    {
        gridLayout->removeWidget(currentWidget);
    }

    _count = 0;
    _width = width;
    for (QWidget* currentWidget : widgetsList)
    {
        gridLayout->addWidget(currentWidget, _count / _width, _count % _width, 1, 1);
        _count++;
    }
}

ToolbarButton* ToolbarGroup::addButton(const char* buttonName, const char* buttonIcon) {
    ToolbarButton* button = new ToolbarButton(buttonName, buttonIcon);
    addButton(button);
    return button;
}

ToolbarButton* ToolbarGroup::addButton(const char* buttonName, const char* buttonIcon, kaguya::LuaRef callback) {
    ToolbarButton* button = new ToolbarButton(buttonName, buttonIcon, callback);
    addButton(button);
    return button;
}

void ToolbarGroup::addWidget(QWidget* widget) {
    auto gridLayout = qobject_cast<QGridLayout*>(this->layout());
    gridLayout->addWidget(widget, _count / _width, _count % _width, 1, 1);
    _count++;
}

std::string ToolbarGroup::label() const {
    return this->title().toStdString();
}

void ToolbarGroup::setLabel(const char* newGroupName) {
    this->setTitle(newGroupName);
}

ToolbarButton* ToolbarGroup::buttonByName(const char* buttonName) {
    auto nbButtons = this->layout()->count();
    std::string buttonText = std::string(buttonName);

    for (int i = 0; i < nbButtons; i++) {
        auto button = qobject_cast<ToolbarButton*>(this->layout()->itemAt(i)->widget());

        if (button != nullptr && button->label() == buttonText) {
            return button;
        }
    }

    return nullptr;
}

std::vector<ToolbarButton*> ToolbarGroup::buttons() {
    auto nbButtons = this->layout()->count();
    std::vector<ToolbarButton*> buttons;

    for (int i = 0; i < nbButtons; i++) {
        auto button = qobject_cast<ToolbarButton*>(this->layout()->itemAt(i)->widget());

        if (button != nullptr) {
            buttons.push_back(button);
        }
    }

    return buttons;
}

void ToolbarGroup::removeButton(ToolbarButton* button) {
    auto gridLayout = qobject_cast<QGridLayout*>(this->layout());
    gridLayout->removeWidget(button);
    delete button;
}

void ToolbarGroup::removeButton(const char* buttonName) {
    removeButton(buttonByName(buttonName));
}

void ToolbarGroup::remove() {
    emit removeGroup(this);
}

void ToolbarGroup::setNonButtonGroup(bool nonButtonGroupIn) {
    _nonButtonGroup = nonButtonGroupIn;
}

bool ToolbarGroup::nonButtonGroup() const {
    return _nonButtonGroup;
}

int ToolbarGroup::width() const {
    return _width;
}

void ToolbarGroup::clear() {
    std::vector<ToolbarButton*> buttonList = buttons();

    for (lc::ui::api::ToolbarButton* button : buttonList) {
        auto gridLayout = qobject_cast<QGridLayout*>(this->layout());
        gridLayout->removeWidget(button);
        button->setVisible(false);
        button->setParent(nullptr);
    }
}
