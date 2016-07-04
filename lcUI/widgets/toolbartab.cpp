#include "toolbartab.h"
#include "ui_toolbartab.h"

ToolbarTab::ToolbarTab(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::ToolbarTab) {
	ui->setupUi(this);

	if(ui->container->layout() == 0) {
		_layout = new QHBoxLayout();
		ui->container->setLayout(_layout);
	}
	else {
		_layout = dynamic_cast<QHBoxLayout*>(ui->container->layout());

		if(!_layout) {
			throw "Wrong layout for ToolbarTab container";
		}
	}
}

QGroupBox* ToolbarTab::addGroup(const char* name) {
	auto group = new QGroupBox(tr(name));
	group->setLayout(new QGridLayout());

	_layout->insertWidget(_layout->count() - 1, group);

	return group;
}

void ToolbarTab::addButton(QGroupBox *groupBox, QPushButton *button, int x, int y, int w, int h) {
	if(groupBox == nullptr) {
		return;
	}

	auto gridLayout = dynamic_cast<QGridLayout*>(groupBox->layout());
	
	if(gridLayout) {
		gridLayout->addWidget(button, y, x, h, w);
	}
	else {
		groupBox->layout()->addWidget(button);
	}
}

QPushButton* ToolbarTab::addButtonStr(QGroupBox* groupBox, const char* buttonName, int x, int y, int w, int h) {
	auto button = new QPushButton(buttonName);

	addButton(groupBox, button, x, y, w, h);

	return button;
}

QGroupBox* ToolbarTab::groupByName(const char* groupName) {
	auto nbGroups = _layout->count();

	for (int i = 0; i < nbGroups; i++) {
		auto groupBox = dynamic_cast<QGroupBox*>(_layout->itemAt(i)->widget());

		if (groupBox && groupBox->title() == groupName) {
			return groupBox;
		}
	}

	return nullptr;
}

QPushButton *ToolbarTab::buttonByText(QGroupBox* groupBox, const char* buttonText) {
	auto nbButtons = groupBox->layout()->count();

	for (int i = 0; i < nbButtons; i++) {
		auto button = dynamic_cast<QPushButton*>(groupBox->layout()->itemAt(i)->widget());

		if(button && button->text() == buttonText) {
			return button;
		}
	}

	return nullptr;
}

void ToolbarTab::removeGroup(QGroupBox *group) {
	if(group == nullptr) {
		return;
	}

	delete group;
}

void ToolbarTab::removeButton(QPushButton *button) {
	if(button != nullptr) {
		delete button;
	}
}