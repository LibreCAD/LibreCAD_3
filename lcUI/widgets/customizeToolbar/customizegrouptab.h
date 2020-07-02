#pragma once

#include <QWidget>
#include "operationdropmodel.h"

#include <widgets/guiAPI/toolbargroup.h>
#include <widgets/guiAPI/toolbarbutton.h>

namespace lc {
    namespace ui {
        namespace widgets {
            /**
            * \brief Represents a group of a toolbar tab in the widget
            */
            class CustomizeGroupTab : public QWidget
            {
                Q_OBJECT
            public:
                /**
                * \brief CustomizeGroupTab constructor
                * \param QString groupName
                * \param pointer to qwidget parent
                */
                CustomizeGroupTab(QString groupName, int width = 3, QWidget* parent = nullptr);

                /**
                * \brief CustomizeGroupTab constructor
                * \param pointer to ToolbarGroup toolbargroup
                * \param pointer to qwidget parent
                */
                CustomizeGroupTab(lc::ui::api::ToolbarGroup* toolbarGroup, QWidget* parent = nullptr);

                /**
                * \brief get label of toolbar tab
                */
                std::string label() const;

                /**
                * \brief Get names of all buttons in group
                * \return List of QString of all button names in group
                */
                QList<QString> buttonNames() const;

                /**
                * \brief Get group width i.e. no. of columns in group
                * \return int width
                */
                int groupWidth() const;

                /**
                * \brief Remove all buttons from group model
                */
                void clearContents();

                /**
                * \brief Add toolbar button
                * \param pointer to qtoolbarbutton
                */
                void addButton(lc::ui::api::ToolbarButton* button);

                /**
                * \brief Set group width
                * \param int width
                */
                void setWidth(int width, int numElements);

            private:
                OperationDropModel* dropModel;
                std::string _groupName;
            };
        }
    }
}
