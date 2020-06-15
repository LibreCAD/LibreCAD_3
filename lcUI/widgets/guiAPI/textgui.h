#pragma once

#include "inputgui.h"
#include <QObject>
#include <QMetaObject>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
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

                void addFinishCallback(kaguya::LuaRef cb);

                void addOnChangeCallback(kaguya::LuaRef cb);

                void setLabel(const std::string& newLabel) override;

                std::string text() const;

                void setText(const std::string& newText);

            public slots:
                void editingFinishedCallbacks();

                void textChangedCallbacks(const QString& changedText);

            private:
                Ui::TextGUI* ui;
                QLabel* _textLabel;
                QLineEdit* _lineEdit;
                std::vector<kaguya::LuaRef> _callbacks_finished;
                std::vector<kaguya::LuaRef> _callbacks_onchange;
            };
        }
    }
}
