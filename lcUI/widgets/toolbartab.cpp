#include "toolbartab.h"
#include "ui_toolbartab.h"

using namespace lc::ui::widgets;

ToolbarTab::ToolbarTab(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::ToolbarTab) {
	ui->setupUi(this);

	if(ui->container->layout() == nullptr) {
		_layout = new QHBoxLayout();
		ui->container->setLayout(_layout);
	}
	else {
		_layout = dynamic_cast<QHBoxLayout*>(ui->container->layout());

		if(_layout == nullptr) {
			throw std::runtime_error("Wrong layout for ToolbarTab container");
		}
	}
}

void ToolbarTab::addWidget(QGroupBox* groupBox, QWidget* widget, int x, int y, int w, int h) {
    if (groupBox == nullptr) {
        return;
    }

    auto gridLayout = dynamic_cast<QGridLayout*>(groupBox->layout());

    if (gridLayout != nullptr) {
        gridLayout->addWidget(widget, y, x, h, w);
    }
    else {
        groupBox->layout()->addWidget(widget);
    }
}

void ToolbarTab::addGroup(api::ToolbarGroup* group) {
    if (group == nullptr) {
        return;
    }

    if (luaInterface != nullptr) {
        group->setLuaInterface(luaInterface);
    }

    _layout->insertWidget(_layout->count() - 1, group);
}

lc::ui::api::ToolbarGroup* ToolbarTab::addGroup(const char* name) {
    lc::ui::api::ToolbarGroup* group = new lc::ui::api::ToolbarGroup(name);
    addGroup(group);
    return group;
}

void ToolbarTab::addButton(api::ToolbarButton* button, const char* groupName) {
    if (groupByName(groupName) == nullptr) {
        addGroup(new api::ToolbarGroup(groupName));
    }

    api::ToolbarGroup* group = static_cast<api::ToolbarGroup*>(groupByName(groupName));
    group->addButton(button);
}

QGroupBox* ToolbarTab::groupByName(const char* groupName) {
	auto nbGroups = _layout->count();

	for (int i = 0; i < nbGroups; i++) {
		auto groupBox = dynamic_cast<QGroupBox*>(_layout->itemAt(i)->widget());

		if (groupBox != nullptr && groupBox->title() == groupName) {
			return groupBox;
		}
	}

	return nullptr;
}

QPushButton *ToolbarTab::buttonByText(QGroupBox* groupBox, const char* buttonText) {
	auto nbButtons = groupBox->layout()->count();

	for (int i = 0; i < nbButtons; i++) {
		auto button = dynamic_cast<QPushButton*>(groupBox->layout()->itemAt(i)->widget());

		if(button != nullptr && button->text() == buttonText) {
			return button;
		}
	}

	return nullptr;
}

void ToolbarTab::removeGroup(const char* groupName) {
    removeGroup(groupByName(groupName));
}

void ToolbarTab::removeGroup(QGroupBox *group) {
	delete group;
}

void ToolbarTab::removeButton(QPushButton *button) {
	delete button;
}

void ToolbarTab::setLuaInterface(LuaInterface* luaInterfaceIn) {
    if (luaInterface != nullptr) {
        return;
    }

    luaInterface = luaInterfaceIn;

    auto nbGroups = _layout->count();
    for (int i = 0; i < nbGroups; i++) {
        auto groupBox = dynamic_cast<api::ToolbarGroup*>(_layout->itemAt(i)->widget());

        if (groupBox != nullptr) {
            groupBox->setLuaInterface(luaInterface);
        }
    }
}
