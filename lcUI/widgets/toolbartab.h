#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>

#include <iostream>

namespace Ui {
	class ToolbarTab;
}

class ToolbarTab : public QWidget {
	public:
		ToolbarTab(QWidget* parent = 0);

		QGroupBox* addGroup(const char* name);
		void addButton(QGroupBox* groupBox, QPushButton* buttonName, int x = 0, int y = 0, int w = 1, int h = 1);
		QPushButton* addButtonStr(QGroupBox* groupBox, const char* buttonName, int x = 0, int y = 0, int w = 1, int h = 1);


		QGroupBox* groupByName(const char* groupName);
		QPushButton* buttonByText(QGroupBox* groupBox, const char* buttonText);

		void removeGroup(QGroupBox* group);
		void removeButton(QPushButton* button);

	private:
		Ui::ToolbarTab* ui;
		QHBoxLayout* _layout;
};