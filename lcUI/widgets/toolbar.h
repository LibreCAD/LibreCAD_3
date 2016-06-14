#pragma once

#include <QDockWidget>

namespace Ui {
	class Toolbar;
}

class Toolbar : public QDockWidget {
	public:
		Toolbar(QWidget *parent = 0);
		~Toolbar();

		void addTab(const char* name, QWidget* page);
		void removeTab(QWidget* page);

		QWidget* tabByName(const char* name);

	private:
		Ui::Toolbar *ui;
};