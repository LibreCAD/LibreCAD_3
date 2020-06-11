#include "toolbargroup.h"

using namespace lc::ui::api;

ToolbarGroup::ToolbarGroup(const char* groupName, int width, QWidget* parent) 
    :
    QGroupBox(groupName, parent),
    width(width),
    _connected(false),
    count(0)
{
    this->setLayout(new QGridLayout());
}

void ToolbarGroup::addButton(ToolbarButton* button) {
    auto gridLayout = qobject_cast<QGridLayout*>(this->layout());
    gridLayout->addWidget(button, count / width, count % width, 1, 1);

    QObject::connect(button, SIGNAL(removeButton(ToolbarButton*)), this, SLOT(removeButton(ToolbarButton*)));
    QObject::connect(button, SIGNAL(connectToCallback(int,ToolbarButton*)), this, SLOT(connectToolbarButtonToCallback(int,ToolbarButton*)));

    if (_connected) {
        button->enableConnections();
    }
    count++;
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
    gridLayout->addWidget(widget, count / width, count % width, 1, 1);
    count++;
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

std::vector<ToolbarButton*> ToolbarGroup::getAllButtons() {
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

void ToolbarGroup::connectToolbarButtonToCallback(int cb_index, ToolbarButton* button) {
    if (button->checkable()) {
        emit connectToCallbackButton(button, "toggled(bool)", button->getCallback(cb_index));
    }
    else {
        emit connectToCallbackButton(button, "pressed()", button->getCallback(cb_index));
    }
}

void ToolbarGroup::enableConnections() {
    if (_connected) {
        return;
    }

    _connected = true;

    auto nbButtons = this->layout()->count();
    for (int i = 0; i < nbButtons; i++) {
        auto button = qobject_cast<ToolbarButton*>(this->layout()->itemAt(i)->widget());
        if (button != nullptr) {
            button->enableConnections();
        }
    }
}
