#pragma once

#include <QDockWidget>
#include "toolbartab.h"

namespace Ui {
	class Toolbar;
}

/**
 * \brief Toolbar widget, displays only ToolbarTabs
 */
class Toolbar : public QDockWidget {
	public:
		/**
		 * \brief Create widget
		 * \param parent Pointer to parent widget
		 */
		Toolbar(QWidget *parent = 0);
		~Toolbar();

		/**
		 * \brief Add a new tab
		 * \param name Tab name
		 * \param page Pointer to ToolbarTab
		 */
		void addTab(const char* name, ToolbarTab* page);

		/**
		 * \brief Remove a tab
		 * \param page Pointer to tab widget
		 */
		void removeTab(QWidget* page);

		/**
		 * \brief Get existing tab
		 * \param name Tab name
		 * \return Pointer to ToolbarTab
		 */
		ToolbarTab* tabByName(const char* name);

	private:
		Ui::Toolbar *ui;
};