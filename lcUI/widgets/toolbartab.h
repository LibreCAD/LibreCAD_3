#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>

#include "guiAPI/toolbarbutton.h"
#include "guiAPI/toolbargroup.h"

namespace Ui {
	class ToolbarTab;
}

namespace lc {
	namespace ui {
		namespace widgets {
            /**
             * \brief Widget used as tab for toolbar containing buttons
             */
			class ToolbarTab : public QWidget {
				public:
					/**
                     * \brief Create widget
                     * \param parent Pointer to parent widget
                     */
					ToolbarTab(QWidget* parent = 0);

					/**
                     * \brief Add new group
                     * \param name Group name
                     * \return Pointer to created QGroupBox
                     */
					QGroupBox* addGroup(const char* name);

                    void addToolbarGroup(api::ToolbarGroup* toolbarGroup);

					/**
                     * \brief Add new widget
                     * \param groupBox Pointer to target group
                     * \param button Pointer to existing QPushButton
                     * \param x Horizontal position
                     * \param y Vertical position
                     * \param w Width
                     * \param h Height
                     */
					void addWidget(QGroupBox* groupBox, QWidget* widget, int x = 0, int y = 0, int w = 0, int h = 1);

					/**
                     * \brief Create a new button and add it to the page
                     * \param groupBox Pointer to target group
                     * \param buttonName Name of the new button
                     * \param x Horizontal position
                     * \param y Vertical position
                     * \param w Width
                     * \param h Height
                     */
					QPushButton*
					addButton(QGroupBox* groupBox, const char* buttonName, int x = 0, int y = 0, int w = 1, int h = 1);

                    void addToolbarButton(QGroupBox* groupBox, api::ToolbarButton* button, int x = 0, int y = 0, int w = 1, int h = 1);

					/**
                     * \brief Get existing group
                     * \param name Tab name
                     * \return Pointer to QGroupBox
                     */
					QGroupBox* groupByName(const char* groupName);

					/**
                     * \brief Get existing button
                     * \param groupBox Group to search
                     * \param buttonText Button name
                     * \return Pointer to QPushButton
                     */
					QPushButton* buttonByText(QGroupBox* groupBox, const char* buttonText);

					/**
                     * \brief Remove group
                     * \param group Pointer to QGroupBox
                     */
					void removeGroup(QGroupBox* group);

					/**
                     * \brief Remove button
                     * \param button Pointer to QPushButton
                     */
					void removeButton(QPushButton* button);

				private:
					Ui::ToolbarTab* ui;
					QHBoxLayout* _layout;
			};
		}
	}
}
