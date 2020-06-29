#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>

#include "toolbarbutton.h"
#include "toolbargroup.h"

#include <kaguya/kaguya.hpp>

namespace Ui {
	class ToolbarTab;
}

namespace lc {
	namespace ui {
		namespace api {
            /**
             * \brief Widget used as tab for toolbar containing buttons
             */
			class ToolbarTab : public QWidget {
                    Q_OBJECT

				public:
					/**
                     * \brief Create widget
                     * \param parent Pointer to parent widget
                     */
					ToolbarTab(const char* tabName, QWidget* parent = 0);

					/**
                     * \brief Add new group
                     * \param Pointer to ToolbarGroup
                     */
                    void addGroup(ToolbarGroup* toolbarGroup);

                    /**
                     * \brief Add new group
                     * \param name Group name
                     * \return Pointer to created ToolbarGroup
                     */
                    ToolbarGroup* addGroup(const char* name);

                    /**
                     * \brief Add new button in given group
                     * \param button ToolbarButton, group name
                     */
                    void addButton(ToolbarButton* button, const char* groupName);

					/**
                     * \brief Get existing group
                     * \param name Tab name
                     * \return Pointer to ToolbarGroup
                     */
                    ToolbarGroup* groupByName(const char* groupName);

                    /**
                     * \brief Get list of all groups
                     * \return vector of pointers to ToolbarGroups
                     */
                    std::vector<ToolbarGroup*> groups();

					/**
                     * \brief Get existing button
                     * \param toolbargroup Group to search
                     * \param buttonText Button name
                     * \return Pointer to ToolbarButton
                     */
                    ToolbarButton* buttonByText(ToolbarGroup* groupBox, const char* buttonText);

                    /**
                     * \brief Remove group by name
                     * \param group name
                     */
                    void removeGroup(const char* groupName);

					/**
                     * \brief Remove button
                     * \param button Pointer to QPushButton
                     */
					void removeButton(ToolbarButton* button);

                    /**
                     * \brief Get tab label
                     * \return string label
                     */
                    std::string label() const;

                    /**
                     * \brief Set tab label
                     * \param new label
                     */
                    void setLabel(const char* newLabel);

                    /**
                     * \brief Remove this widget
                     */
                    void remove();

                    /**
                    * \brief Removes all groups (remove and delete) ONLY BUTTON GROUPS
                    */
                    void clear();

                signals:
                    /**
                     * \brief Signal for label being changed
                     */
                    void labelChanged(QString newLabel, QString oldLabel);

                    /**
                     * \brief Signal for removing this tab
                     */
                    void removeTab(lc::ui::api::ToolbarTab* tab);

                public slots:
                    /**
                     * \brief Remove group
                     * \param group Pointer to ToolbarGroup
                     */
                    void removeGroup(ToolbarGroup* group);

				private:
                    std::string _label;
					Ui::ToolbarTab* ui;
					QHBoxLayout* _layout;
			};
		}
	}
}
