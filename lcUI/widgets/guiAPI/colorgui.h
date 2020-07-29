#pragma once

#include <string>

#include "inputgui.h"
#include <cad/meta/color.h>

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QString>

namespace Ui {
    class ColorGUI;
}

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Color Picker GUI Widget
            */
            class ColorGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief ColorGUI constructor
                * \param string gui label
                * \param parent qwidget parent
                */
                ColorGUI(std::string label, QWidget* parent = nullptr);

                /**
                * \brief ColorGUI destructor
                */
                ~ColorGUI();

                /**
                * \brief Add lua value to return table
                * \param LuaRef table
                */
                void getLuaValue(kaguya::LuaRef& table);

                /**
                * \brief Return color value
                * \return lc::Color value
                */
                lc::Color value() const;

                /**
                * \brief Set color value
                * \param lc::Color color
                */
                void setValue(lc::Color col);

                /**
                * \brief Add callback for color selected
                * \param LuaRef callback
                */
                void addCallback(kaguya::LuaRef cb);

                /**
                * \brief Run callbacks for color selected
                */
                void colorSelectedCallbacks();

                /**
                * \brief Hide widget label
                */
                void hideLabel() override;

            public slots:
                /**
                * \brief Slot called when color is changed
                */
                void changeColor();

            protected:
                /**
                * \brief Copy widget value to the clipboard
                */
                void copyValue(QDataStream& stream) override;

                /**
                * \brief Set widget value from clipboard
                */
                void pasteValue(QDataStream& stream) override;

            private:
                QColor getIdealTextColor(const QColor& rBackgroundColor) const;

            private:
                Ui::ColorGUI* ui;
                QLabel* _textLabel;
                QPushButton* _colorButton;
                QColor _color;

                std::vector<kaguya::LuaRef> _callbacks;
            };

        }
    }
}
