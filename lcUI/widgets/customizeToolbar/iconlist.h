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

            private:
                OperationDragModel* opModel;
            };
        }
    }
}
