#pragma once

#include <QWidget>
#include <QListView>

#include "operationdragmodel.h"
#include <widgets/toolbar.h>

namespace lc {
    namespace ui {
        namespace widgets {
            /**
            * \brief List of all icons in the toolbar
            */
            class IconList : public QListView
            {
                Q_OBJECT
            public:
                IconList(QWidget* parent = nullptr);

                /**
                * \brief Add all existing toolbar buttons to the list
                * \param pointer to Toolbar toolbar
                */
                void initialize(Toolbar* toolbar);

                /**
                * \brief Event of button being dropped on area
                */
                void dropEvent(QDropEvent* e) override;

                /**
                * \brief Get button of given name
                * \param QString button name
                * \return pointer to toolbar button
                */
                lc::ui::api::ToolbarButton* buttonByName(QString& buttonName);

            private:
                OperationDragModel* opModel;
            };
        }
    }
}
