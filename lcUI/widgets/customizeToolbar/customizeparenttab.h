#pragma once

#include <QWidget>
#include <QTabWidget>

namespace lc {
    namespace ui {
        namespace widgets {
            class CustomizeParentTab : public QTabWidget
            {
            public:
                CustomizeParentTab(QString label, QWidget* parent = nullptr);

            public slots:
                void addGroupTab();
            };
        }
    }
}
