#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>

#include "guiAPI/toolbarbutton.h"
#include "guiAPI/toolbargroup.h"

#include <luainterface.h>

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
                     * \brief Add new group
                     * \param name Group name
                     * \return Pointer to created QGroupBox
                     */

                    void addGroup(api::ToolbarGroup* toolbarGroup);

                    lc::ui::api::ToolbarGroup* addGroup(const char* name);

                    void addButton(api::ToolbarButton* button, const char* groupName);

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
                     * \brief Remove group by name
                     * \param group Pointer to QGroupBox
                     */
                    void removeGroup(const char* groupName);

					/**
                     * \brief Remove button
                     * \param button Pointer to QPushButton
                     */
					void removeButton(QPushButton* button);

                    void setLuaInterface(LuaInterface* luaInterfaceIn);

				private:
					Ui::ToolbarTab* ui;
					QHBoxLayout* _layout;
                    LuaInterface* luaInterface;
			};
		}
	}
}
