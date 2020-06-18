#pragma once

#include "inputgui.h"
#include <QObject>
#include <QMetaObject>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>

#include "cad/geometry/geocoordinate.h"

namespace Ui {
    class CoordinateGUI;
}

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Coordinate GUI Widget
            */
            class CoordinateGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief CoordinateGUI constructor
                * \param string gui label
                * \param parent qwidget parent
                */
                CoordinateGUI(std::string label, QWidget* parent = nullptr);

                /**
                * \brief CoordinateGUI destructor
                */
                ~CoordinateGUI();

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
                * \brief Update the coordinate variable
                */
                void updateCoordinate();

                /**
                * \brief Update the GUI tooltip
                */
                void updateCoordinateDisplay();

                /**
                * \brief Overriden Angle Text GUI widget label
                * \param string gui label
                */
                void setLabel(const std::string& newLabel) override;

                /**
                * \brief Return coordinate
                * \return geo::Coordinate coordinate
                */
                lc::geo::Coordinate value() const;

                /**
                * \brief Set coordinate value
                * \param geo::Coordinate coordinate
                */
                void setValue(lc::geo::Coordinate coord);

                /**
                * \brief Return lua value
                * \param LuaRef value
                */
                void getLuaValue(kaguya::LuaRef& table) override;

            public slots:
                /**
                * \brief Run callbacks for editing finished
                */
                void editingFinishedCallbacks();

                /**
                * \brief Run callbacks for text changed
                */
                void textChangedCallbacks();

            private:
                /**
                * \brief Helper function to generate the required tooltip
                * \return QString tooltip
                */
                QString generateTooltip() const;

            private:
                Ui::CoordinateGUI* ui;
                QLineEdit* _xcoordEdit;
                QLineEdit* _ycoordEdit;
                QLabel* _textLabel;
                lc::geo::Coordinate _coordinate;

                std::vector<kaguya::LuaRef> _callbacks_finished;
                std::vector<kaguya::LuaRef> _callbacks_onchange;
            };

        }
    }
}
