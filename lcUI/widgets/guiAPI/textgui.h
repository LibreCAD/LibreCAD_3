#pragma once

#include "inputgui.h"
#include <QObject>
#include <QMetaObject>
#include <QWidget>
#include <QTextEdit>
#include <QHBoxLayout>

namespace Ui { 
    class TextGUI;
}

namespace lc {
    namespace ui {
        namespace api {
            class TextGUI : public InputGUI
            {
                Q_OBJECT

            public:
                TextGUI(std::string label, QWidget* parent = nullptr);
                ~TextGUI();

                void setLabel(const std::string& newLabel) override;

            private:
                Ui::TextGUI* ui;
            };
        }
    }
}
