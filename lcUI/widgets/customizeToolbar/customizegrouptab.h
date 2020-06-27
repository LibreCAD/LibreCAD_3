#pragma once

#include <QWidget>
#include "operationdropmodel.h"

namespace lc {
    namespace ui {
        namespace widgets {
            class CustomizeGroupTab : public QWidget
            {
                Q_OBJECT
            public:
                explicit CustomizeGroupTab(QString groupName, QWidget* parent = nullptr);

            signals:

            private:
                OperationDropModel* dropModel;
                QString groupName;
            };
        }
    }
}
