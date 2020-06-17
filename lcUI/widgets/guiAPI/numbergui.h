#pragma once

#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include "inputgui.h"

namespace Ui { 
    class NumberGUI;
}

namespace lc
{
    namespace ui
    {
        namespace api
        {
            class NumberGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief NumberGUI constructor
                * \param string gui label
                * \param double minimum value
                * \param double maximum value
                * \param parent qwidget parent
                */
                NumberGUI(std::string label, double minVal=-10000, double maxVal=10000, QWidget* parent = nullptr);

                /**
                * \brief NumberGUI destructor
                */
                ~NumberGUI();

                /**
                * \brief Add lua callbacks for value changed signal
                * \param LuaRef callback
                */
                void addCallback(kaguya::LuaRef cb);

                /**
                * \brief Overriden set SLider GUI widget label
                * \param string gui label
                */
                void setLabel(const std::string& newLabel) override;

                /**
                * \brief Return number
                * \return double value
                */
                double value() const;

                /**
                * \brief Set number value
                * \param double value
                */
                void setValue(double val);

            public slots:
                /**
                * \brief Run value changed callbacks
                * \param double changed value
                */
                void valueChangedCallbacks(double val);

            private:
                Ui::NumberGUI* ui;
                QLabel* _textLabel;
                QDoubleSpinBox* _spinBox;

                std::vector<kaguya::LuaRef> _callbacks;
            };
        }
    }
}
