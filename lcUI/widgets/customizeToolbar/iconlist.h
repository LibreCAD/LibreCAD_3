#pragma once

#include <QWidget>
#include <QListView>

#include "operationdragmodel.h"
#include <widgets/toolbar.h>

namespace lc {
    namespace ui {
        namespace widgets {
            class IconList : public QListView
            {
                Q_OBJECT
            public:
                IconList(QWidget* parent = nullptr);

                void initialize(Toolbar* toolbar);

                void dropEvent(QDropEvent* e) override;

            private:
                OperationDragModel* opModel;
            };
        }
    }
}
