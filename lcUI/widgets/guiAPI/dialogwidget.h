#pragma once

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>

#include "inputgui.h"

namespace Ui { 
    class DialogWidget; 
}

namespace lc {
    namespace ui {
        namespace api {
            class DialogWidget : public QDialog
            {
                Q_OBJECT

            public:
                DialogWidget(QWidget* parent = nullptr);
                ~DialogWidget();

                void addWidget(InputGUI* guiWidget);

            private:
                Ui::DialogWidget* ui;
                QVBoxLayout* vboxlayout;
            };
        }
    }
}
