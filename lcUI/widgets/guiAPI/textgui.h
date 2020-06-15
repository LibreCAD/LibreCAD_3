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
            /**
            * \brief Text GUI Widget
            */
            class TextGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief TextGUI constructor
                * \param string gui label
                * \param parent qwidget parent
                */
                TextGUI(std::string label, QWidget* parent = nullptr);
                
                /**
                * \brief TextGUI destructor
                */
                ~TextGUI();

                /**
                * \brief Add callback for editingFinished
                * \param LuaRef lua callback
                */
                void addFinishCallback(kaguya::LuaRef cb);

                /**
                * \brief Add callback for textChanged
                * \param LuaRef lua callback
                */
                void addOnChangeCallback(kaguya::LuaRef cb);

                /**
                * \brief Overriden Set Text GUI widget label
                * \param string gui label
                */
                void setLabel(const std::string& newLabel) override;

                /**
                * \brief Return lineedit text
                * \return string text
                */
                std::string text() const;

                /**
                * \brief Set lineedit text
                * \param string text
                */
                void setText(const std::string& newText);

            public slots:
                /**
                * \brief Run callbacks for editing finished
                */
                void editingFinishedCallbacks();

                /**
                * \brief Run callbacks for text changed
                */
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
