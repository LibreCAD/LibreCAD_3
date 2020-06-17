#pragma once

#include "inputgui.h"
#include <QWidget>
#include <QLabel>
#include <QComboBox>

namespace Ui { 
    class ComboBoxGUI;
}

namespace lc
{
    namespace ui
    {
        namespace api
        {
            /**
            * \brief ComboBox GUI Widget
            */
            class ComboBoxGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief ComboBoxGUI constructor
                * \param string gui label
                * \param parent qwidget parent
                */
                ComboBoxGUI(std::string label, QWidget* parent = nullptr);

                /**
                * \brief TextGUI destructor
                */
                ~ComboBoxGUI();

                /**
                * \brief Add callback for activated signal
                * \param LuaRef lua callback
                */
                void addCallback(kaguya::LuaRef cb);

                /**
                * \brief Overriden Set ComboBox GUI widget label
                * \param string gui label
                */
                void setLabel(const std::string& newLabel) override;

                /**
                * \brief Add ComboBox string item
                * \param string item
                * \param int index to insert at, default=end oflist
                */
                void addItem(const std::string& item, int index=-1);

            public slots:
                /**
                * \brief Run callbacks
                * \param int item index
                */
                void activatedCallbacks(int index);

            private:
                Ui::ComboBoxGUI* ui;
                QLabel* _textLabel;
                QComboBox* _comboBox;

                std::vector<kaguya::LuaRef> _activatedCallbacks;
            };
        }
    }
}
