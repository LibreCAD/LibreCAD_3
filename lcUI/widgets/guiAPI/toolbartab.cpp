#include "toolbartab.h"
#include "ui_toolbartab.h"

#include <widgets/toolbar.h>

using namespace lc::ui::api;

ToolbarTab::ToolbarTab(const char* tabName, QWidget *parent) :
		QWidget(parent),
        _label(tabName),
        _connected(false),
		ui(new Ui::ToolbarTab) {
	ui->setupUi(this);

	if(ui->container->layout() == nullptr) {
		_layout = new QHBoxLayout();
		ui->container->setLayout(_layout);
	}
	else {
		_layout = qobject_cast<QHBoxLayout*>(ui->container->layout());

		if(_layout == nullptr) {
			throw std::runtime_error("Wrong layout for ToolbarTab container");
		}
	}
}

void ToolbarTab::addGroup(ToolbarGroup* group) {
    if (group == nullptr) {
        return;
    }

    if (_connected) {
        group->enableConnections();
    }

    _layout->insertWidget(_layout->count() - 1, group);
    lc::ui::widgets::Toolbar* toolbarWidget = qobject_cast<lc::ui::widgets::Toolbar*>(this->parentWidget()->parentWidget()->parentWidget());

    if (toolbarWidget == nullptr) {
        return;
    }

    QObject::connect(group, SIGNAL(removeGroup(ToolbarGroup*)), this, SLOT(removeGroup(ToolbarGroup*)));
    QObject::connect(group, SIGNAL(connectToCallbackButton(lc::ui::api::ToolbarButton*, const std::string&, kaguya::LuaRef&)), toolbarWidget, SLOT(connectToCallbackToolbar(lc::ui::api::ToolbarButton*, const std::string&, kaguya::LuaRef&)));
    QObject::connect(group, SIGNAL(disconnectCallbackButton(lc::ui::api::ToolbarButton*, const std::string&, kaguya::LuaRef&)), toolbarWidget, SLOT(disconnectCallbackToolbar(lc::ui::api::ToolbarButton*, const std::string&, kaguya::LuaRef&)));
}

ToolbarGroup* ToolbarTab::addGroup(const char* name) {
    ToolbarGroup* group = new ToolbarGroup(name);
    addGroup(group);
    return group;
}

void ToolbarTab::addButton(ToolbarButton* button, const char* groupName) {
    if (groupByName(groupName) == nullptr) {
        addGroup(new ToolbarGroup(groupName));
    }

    ToolbarGroup* group = qobject_cast<ToolbarGroup*>(groupByName(groupName));
    group->addButton(button);
}

ToolbarGroup* ToolbarTab::groupByName(const char* groupName) {
	auto nbGroups = _layout->count();

	for (int i = 0; i < nbGroups; i++) {
		auto groupBox = qobject_cast<ToolbarGroup*>(_layout->itemAt(i)->widget());

		if (groupBox != nullptr && groupBox->title() == groupName) {
			return groupBox;
		}
	}

	return nullptr;
}

std::vector<ToolbarGroup*> ToolbarTab::getAllGroups() {
    std::vector<ToolbarGroup*> groupsList;
    auto nbGroups = _layout->count();

    for (int i = 0; i < nbGroups; i++) {
        auto groupBox = qobject_cast<ToolbarGroup*>(_layout->itemAt(i)->widget());

        if (groupBox != nullptr) {
            groupsList.push_back(groupBox);
        }
    }

    return groupsList;
}

ToolbarButton* ToolbarTab::buttonByText(ToolbarGroup* groupBox, const char* buttonText) {
	auto nbButtons = groupBox->layout()->count();

	for (int i = 0; i < nbButtons; i++) {
		auto button = qobject_cast<ToolbarButton*>(groupBox->layout()->itemAt(i)->widget());

		if(button != nullptr && button->label() == buttonText) {
			return button;
		}
	}

	return nullptr;
}

void ToolbarTab::removeGroup(const char* groupName) {
    removeGroup(groupByName(groupName));
}

void ToolbarTab::removeGroup(ToolbarGroup* group) {
    if (group != nullptr) {
        _layout->removeWidget(group);
        delete group;
    }
}

void ToolbarTab::removeButton(ToolbarButton* button) {
    if (button != nullptr) {
        delete button;
    }
}

std::string ToolbarTab::label() const {
    return _label;
}

void ToolbarTab::setLabel(const char* newLabel) {
    QString oldLabel = QString(_label.c_str());
    _label = std::string(newLabel);

    // set tab label
    emit labelChanged(QString(newLabel), oldLabel);
}

void ToolbarTab::remove() {
    emit removeTab(this);
}

void ToolbarTab::enableConnections() {
    if (_connected) {
        return;
    }

    _connected = true;

    auto nbGroups = _layout->count();
    for (int i = 0; i < nbGroups; i++) {
        auto groupBox = qobject_cast<ToolbarGroup*>(_layout->itemAt(i)->widget());

        if (groupBox != nullptr) {
            groupBox->enableConnections();
        }
    }
}
