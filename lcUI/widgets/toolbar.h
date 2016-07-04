#pragma once

#include <QDockWidget>
#include "toolbartab.h"

namespace Ui {
	class Toolbar;
}

class Toolbar : public QDockWidget {
	public:
		Toolbar(QWidget *parent = 0);
		~Toolbar();

		void addTab(const char* name, ToolbarTab* page);
		void removeTab(QWidget* page);

		ToolbarTab* tabByName(const char* name);

	private:
		Ui::Toolbar *ui;
};