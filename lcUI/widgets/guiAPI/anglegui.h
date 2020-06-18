#pragma once

#include <string>

#include "inputgui.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace Ui {
    class AngleGUI;
}

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Angle GUI Widget
            */
            class AngleGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief AngleGUI constructor
                * \param string gui label
                * \param parent qwidget parent
                */
                AngleGUI(std::string label, QWidget* parent = nullptr);

                /**
                * \brief TextGUI destructor
                */
                ~AngleGUI();

                /**
                * \brief Convert displayed angle to degrees
                */
                void toDegrees();

                /**
                * \brief Convert displayed angle to radians
                */
                void toRadians();

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
                * \brief Overriden Angle Text GUI widget label
                * \param string gui label
                */
                void setLabel(const std::string& newLabel) override;

                /**
                * \brief Return angle
                * \return double value
                */
                double value() const;

                /**
                * \brief Set angle value
                * \param double value
                */
                void setValue(double val);

                /**
                * \brief Return lua value
                * \param LuaRef value
                */
                void getLuaValue(kaguya::LuaRef& table) override;

            public slots:
                /**
                * \brief Toggle the units from rad to deg and vice versa
                */
                void toggleUnit();

                /**
                * \brief Run callbacks for editing finished
                */
                void editingFinishedCallbacks();

                /**
                * \brief Run callbacks for text changed
                */
                void textChangedCallbacks(const QString& changedText);

            protected:
                enum class AngleType {
                    Degrees,
                    Radians
                };

                AngleType angleType;

            private:
                Ui::AngleGUI* ui;
                QLabel* _textLabel;
                QPushButton* _angleTypeButton;
                QLineEdit* _lineEdit;
                std::vector<kaguya::LuaRef> _callbacks_finished;
                std::vector<kaguya::LuaRef> _callbacks_onchange;
            };

        }
    }
}
