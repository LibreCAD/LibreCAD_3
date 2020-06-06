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
					ToolbarTab(const char* tabName, QWidget* parent = 0);

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
                    lc::ui::api::ToolbarGroup* groupByName(const char* groupName);

					/**
                     * \brief Get existing button
                     * \param toolbargroup Group to search
                     * \param buttonText Button name
                     * \return Pointer to ToolbarButton
                     */
                    lc::ui::api::ToolbarButton* buttonByText(lc::ui::api::ToolbarGroup* groupBox, const char* buttonText);

					/**
                     * \brief Remove group
                     * \param group Pointer to QGroupBox
                     */
					void removeGroup(lc::ui::api::ToolbarGroup* group);

                    /**
                     * \brief Remove group by name
                     * \param group name
                     */
                    void removeGroup(const char* groupName);

					/**
                     * \brief Remove button
                     * \param button Pointer to QPushButton
                     */
					void removeButton(lc::ui::api::ToolbarButton* button);

                    std::string label() const;

                    void setLabel(const char* newLabel);

                    void setLuaInterface(LuaInterface* luaInterfaceIn);

				private:
                    std::string _label;
					Ui::ToolbarTab* ui;
					QHBoxLayout* _layout;
                    LuaInterface* luaInterface;
			};
		}
	}
}
