#pragma once

#include <QWidget>
#include "operationdropmodel.h"

#include <widgets/guiAPI/toolbargroup.h>

namespace lc {
    namespace ui {
        namespace widgets {
            class CustomizeGroupTab : public QWidget
            {
                Q_OBJECT
            public:
                CustomizeGroupTab(QString groupName, QWidget* parent = nullptr);

                CustomizeGroupTab(lc::ui::api::ToolbarGroup* toolbarGroup, QWidget* parent = nullptr);

            private:
                OperationDropModel* dropModel;
                QString groupName;
            };
        }
    }
}
