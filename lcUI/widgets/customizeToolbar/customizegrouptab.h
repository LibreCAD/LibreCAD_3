#pragma once

#include <QWidget>
#include "operationdropmodel.h"

#include <widgets/guiAPI/toolbargroup.h>

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
                CustomizeGroupTab(QString groupName, QWidget* parent = nullptr);

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

            private:
                OperationDropModel* dropModel;
                std::string _groupName;
            };
        }
    }
}
