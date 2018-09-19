#include "toolbartab.h"
#include "ui_toolbartab.h"

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

QGroupBox* ToolbarTab::addGroup(const char* name) {
	auto group = new QGroupBox(tr(name));
	group->setLayout(new QGridLayout());

	_layout->insertWidget(_layout->count() - 1, group);

	return group;
}

QPushButton* ToolbarTab::addButton(QGroupBox* groupBox, const char* buttonName, int x, int y, int w, int h) {
	auto button = new QPushButton(buttonName);

	addWidget(groupBox, button, x, y, w, h);

	return button;
}

void ToolbarTab::addWidget(QGroupBox *groupBox, QWidget *widget, int x, int y, int w, int h) {
	if(groupBox == nullptr) {
		return;
	}

	auto gridLayout = dynamic_cast<QGridLayout*>(groupBox->layout());

    if(gridLayout != nullptr) {
        gridLayout->addWidget(widget, y, x, h, w);
    }
    else {
        groupBox->layout()->addWidget(widget);
    }
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

void ToolbarTab::removeGroup(QGroupBox *group) {
	delete group;
}

void ToolbarTab::removeButton(QPushButton *button) {
	delete button;
}